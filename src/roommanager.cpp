// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2021 Alexey Rusakov <TODO>
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roommanager.h"

#include "chatbarcache.h"
#include "controller.h"
#include "enums/delegatetype.h"
#include "models/messageeventmodel.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include <KLocalizedString>
#include <QDesktopServices>
#include <QQuickTextDocument>
#include <QStandardPaths>

#include <Quotient/csapi/joining.h>
#include <Quotient/csapi/knocking.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/user.h>

#ifndef Q_OS_ANDROID
#include <KIO/OpenUrlJob>
#endif

RoomManager::RoomManager(QObject *parent)
    : QObject(parent)
    , m_currentRoom(nullptr)
    , m_lastCurrentRoom(nullptr)
    , m_config(KSharedConfig::openStateConfig())
    , m_messageEventModel(new MessageEventModel(this))
    , m_messageFilterModel(new MessageFilterModel(this, m_messageEventModel))
    , m_mediaMessageFilterModel(new MediaMessageFilterModel(this, m_messageFilterModel))
{
    m_lastRoomConfig = m_config->group(QStringLiteral("LastOpenRoom"));

    connect(this, &RoomManager::currentRoomChanged, this, [this]() {
        m_messageEventModel->setRoom(m_currentRoom);
    });
}

RoomManager::~RoomManager()
{
}

RoomManager &RoomManager::instance()
{
    static RoomManager _instance;
    return _instance;
}

NeoChatRoom *RoomManager::currentRoom() const
{
    return m_currentRoom;
}

MessageEventModel *RoomManager::messageEventModel() const
{
    return m_messageEventModel;
}

MessageFilterModel *RoomManager::messageFilterModel() const
{
    return m_messageFilterModel;
}

MediaMessageFilterModel *RoomManager::mediaMessageFilterModel() const
{
    return m_mediaMessageFilterModel;
}

void RoomManager::openResource(const QString &idOrUri, const QString &action)
{
    Uri uri{idOrUri};
    if (!uri.isValid()) {
        Q_EMIT warning(i18n("Malformed or empty Matrix id"), i18n("%1 is not a correct Matrix identifier", idOrUri));
        return;
    }
    auto account = Controller::instance().activeConnection();

    if (uri.type() != Uri::NonMatrix) {
        if (!account) {
            return;
        }
        if (!action.isEmpty()) {
            uri.setAction(action);
        }
        // TODO we should allow the user to select a connection.
    }

    const auto result = visitResource(account, uri);
    if (result == Quotient::CouldNotResolve) {
        Q_EMIT warning(i18n("Room not found"), i18n("There's no room %1 in the room list. Check the spelling and the account.", idOrUri));
    } else { // Invalid cases should have been eliminated earlier
        Q_ASSERT(result == Quotient::UriResolved);
    }
}

void RoomManager::maximizeMedia(int index)
{
    if (index < -1 || index > m_mediaMessageFilterModel->rowCount()) {
        return;
    }
    Q_EMIT showMaximizedMedia(index);
}

void RoomManager::requestFullScreenClose()
{
    Q_EMIT closeFullScreen();
}

void RoomManager::viewEventSource(const QString &eventId)
{
    Q_EMIT showEventSource(eventId);
}

void RoomManager::viewEventMenu(const QString &eventId,
                                const QVariantMap &author,
                                DelegateType::Type delegateType,
                                const QString &plainText,
                                const QString &htmlText,
                                const QString &selectedText,
                                const QString &mimeType,
                                const FileTransferInfo &progressInfo)
{
    if (delegateType == DelegateType::Image || delegateType == DelegateType::Video || delegateType == DelegateType::Audio
        || delegateType == DelegateType::File) {
        Q_EMIT showFileMenu(eventId, author, delegateType, plainText, mimeType, progressInfo);
        return;
    }

    Q_EMIT showMessageMenu(eventId, author, delegateType, plainText, htmlText, selectedText);
}

bool RoomManager::hasOpenRoom() const
{
    return m_currentRoom != nullptr;
}

void RoomManager::setUrlArgument(const QString &arg)
{
    m_arg = arg;
}

void RoomManager::loadInitialRoom()
{
    Q_ASSERT(Controller::instance().activeConnection());

    if (!m_arg.isEmpty()) {
        openResource(m_arg);
    }

    if (m_currentRoom) {
        // we opened a room with the arg parsing already
        return;
    }

    openRoomForActiveConnection();

    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, &RoomManager::openRoomForActiveConnection);
}

void RoomManager::openRoomForActiveConnection()
{
    if (!Controller::instance().activeConnection()) {
        return;
    }
    // Read from last open room
    QString roomId = m_lastRoomConfig.readEntry(Controller::instance().activeConnection()->userId(), QString());

    // TODO remove legacy check at some point.
    if (roomId.isEmpty()) {
        roomId = NeoChatConfig::self()->openRoom();
    }

    if (!roomId.isEmpty()) {
        // Here we can cast because the controller has been configured to
        // return NeoChatRoom instead of simple Quotient::Room
        const auto room = qobject_cast<NeoChatRoom *>(Controller::instance().activeConnection()->room(roomId));

        if (room) {
            if (room->isSpace()) {
                enterSpaceHome(room);
            } else {
                enterRoom(room);
            }
        }
    }
}

void RoomManager::enterRoom(NeoChatRoom *room)
{
    if (m_currentRoom && !m_currentRoom->editCache()->editId().isEmpty()) {
        m_currentRoom->editCache()->setEditId({});
    }
    if (m_currentRoom && m_chatDocumentHandler) {
        // We're doing these things here because it is critical that they are switched at the same time
        if (m_chatDocumentHandler->document()) {
            m_currentRoom->mainCache()->setSavedText(m_chatDocumentHandler->document()->textDocument()->toPlainText());
            m_chatDocumentHandler->setRoom(room);
            m_chatDocumentHandler->document()->textDocument()->setPlainText(room->mainCache()->savedText());
        } else {
            m_chatDocumentHandler->setRoom(room);
        }
    }
    m_lastCurrentRoom = std::exchange(m_currentRoom, room);
    Q_EMIT currentRoomChanged();

    if (!m_lastCurrentRoom) {
        Q_EMIT pushRoom(room, QString());
    } else {
        Q_EMIT replaceRoom(m_currentRoom, QString());
    }

    if (room && room->timelineSize() == 0) {
        room->getPreviousContent(20);
    }

    // Save last open room
    m_lastRoomConfig.writeEntry(Controller::instance().activeConnection()->userId(), room->id());
}

void RoomManager::openWindow(NeoChatRoom *room)
{
    // forward the call to QML
    Q_EMIT openRoomInNewWindow(room);
}

void RoomManager::enterSpaceHome(NeoChatRoom *spaceRoom)
{
    if (!spaceRoom->isSpace()) {
        return;
    }
    // If replacing a normal room message timeline make sure any edit is cancelled.
    if (m_currentRoom && !m_currentRoom->editCache()->editId().isEmpty()) {
        m_currentRoom->editCache()->setEditId({});
    }
    // Save the chatbar text for the current room if any before switching
    if (m_currentRoom && m_chatDocumentHandler) {
        if (m_chatDocumentHandler->document()) {
            m_currentRoom->mainCache()->setSavedText(m_chatDocumentHandler->document()->textDocument()->toPlainText());
        }
    }
    m_lastCurrentRoom = std::exchange(m_currentRoom, spaceRoom);
    Q_EMIT currentRoomChanged();

    if (!m_lastCurrentRoom) {
        Q_EMIT pushSpaceHome(spaceRoom);
    } else {
        Q_EMIT replaceSpaceHome(m_currentRoom);
    }

    // Save last open room
    m_lastRoomConfig.writeEntry(Controller::instance().activeConnection()->userId(), spaceRoom->id());
}

UriResolveResult RoomManager::visitUser(User *user, const QString &action)
{
    if (action == "mention"_ls || action.isEmpty()) {
        // send it has QVariantMap because the properties in the
        user->load();
        Q_EMIT showUserDetail(user);
    } else if (action == "_interactive"_ls) {
        user->requestDirectChat();
    } else if (action == "chat"_ls) {
        user->load();
        Q_EMIT askDirectChatConfirmation(user);
    } else {
        return Quotient::IncorrectAction;
    }

    return Quotient::UriResolved;
}

void RoomManager::visitRoom(Room *room, const QString &eventId)
{
    auto neoChatRoom = qobject_cast<NeoChatRoom *>(room);
    Q_ASSERT(neoChatRoom != nullptr);

    if (m_currentRoom) {
        if (m_currentRoom->id() == room->id()) {
            Q_EMIT goToEvent(eventId);
        } else {
            m_lastCurrentRoom = std::exchange(m_currentRoom, neoChatRoom);
            Q_EMIT currentRoomChanged();
            Q_EMIT replaceRoom(neoChatRoom, eventId);
        }
    } else {
        m_lastCurrentRoom = std::exchange(m_currentRoom, neoChatRoom);
        Q_EMIT currentRoomChanged();
        Q_EMIT pushRoom(neoChatRoom, eventId);
    }
}

void RoomManager::joinRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QStringList &viaServers)
{
    auto job = account->joinRoom(QString::fromLatin1(QUrl::toPercentEncoding(roomAliasOrId)), viaServers);
    connectSingleShot(job, &Quotient::BaseJob::finished, this, [this, account](Quotient::BaseJob *finish) {
        if (finish->status() == Quotient::BaseJob::Success) {
            connectSingleShot(account, &Quotient::Connection::newRoom, this, [this](Quotient::Room *room) {
                enterRoom(dynamic_cast<NeoChatRoom *>(room));
            });
        } else {
            Q_EMIT warning(i18n("Failed to join room"), finish->errorString());
        }
    });
}

void RoomManager::knockRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QString &reason, const QStringList &viaServers)
{
    auto *const job = account->callApi<KnockRoomJob>(roomAliasOrId, viaServers, reason);
    // Upon completion, ensure a room object is created in case it hasn't come
    // with a sync yet. If the room object is not there, provideRoom() will
    // create it in Join state. finished() is used here instead of success()
    // to overtake clients that may add their own slots to finished().
    connectSingleShot(job, &BaseJob::finished, this, [this, job, account] {
        if (job->status() == Quotient::BaseJob::Success) {
            connectSingleShot(account, &Quotient::Connection::newRoom, this, [this](Quotient::Room *room) {
                Q_EMIT currentRoom()->showMessage(NeoChatRoom::Info, i18n("You requested to join '%1'", room->name()));
            });
        } else {
            Q_EMIT warning(i18n("Failed to request joining room"), job->errorString());
        }
    });
}

bool RoomManager::visitNonMatrix(const QUrl &url)
{
#ifdef Q_OS_ANDROID
    if (!QDesktopServices::openUrl(url)) {
        Q_EMIT warning(i18n("No application for the link"), i18n("Your operating system could not find an application for the link."));
    }
#else
    auto *job = new KIO::OpenUrlJob(url);
    connect(job, &KJob::finished, this, [this](KJob *job) {
        if (job->error()) {
            Q_EMIT warning(i18n("Could not open URL"), job->errorString());
        }
    });
    job->start();
#endif
    return true;
}

void RoomManager::reset()
{
    m_arg = QString();
    m_currentRoom = nullptr;
    m_lastCurrentRoom = nullptr;
    Q_EMIT currentRoomChanged();
}

void RoomManager::leaveRoom(NeoChatRoom *room)
{
    if (!room) {
        return;
    }
    if (m_lastCurrentRoom && room->id() == m_lastCurrentRoom->id()) {
        m_lastCurrentRoom = nullptr;
    }
    if (m_currentRoom && m_currentRoom->id() == room->id()) {
        m_currentRoom = m_lastCurrentRoom;
        m_lastCurrentRoom = nullptr;
        Q_EMIT currentRoomChanged();
    }

    room->forget();
}

ChatDocumentHandler *RoomManager::chatDocumentHandler() const
{
    return m_chatDocumentHandler;
}

void RoomManager::setChatDocumentHandler(ChatDocumentHandler *handler)
{
    m_chatDocumentHandler = handler;
    m_chatDocumentHandler->setRoom(m_currentRoom);
    Q_EMIT chatDocumentHandlerChanged();
}

#include "moc_roommanager.cpp"
