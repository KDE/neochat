// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2021 Alexey Rusakov <TODO>
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roommanager.h"

#include "chatbarcache.h"
#include "eventhandler.h"
#include "messagecomponenttype.h"
#include "models/timelinemodel.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
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
    , m_timelineModel(new TimelineModel(this))
    , m_messageFilterModel(new MessageFilterModel(this, m_timelineModel))
    , m_mediaMessageFilterModel(new MediaMessageFilterModel(this, m_messageFilterModel))
{
    m_lastRoomConfig = m_config->group(QStringLiteral("LastOpenRoom"));
    m_lastSpaceConfig = m_config->group(QStringLiteral("LastOpenSpace"));
    m_directChatsConfig = m_config->group(QStringLiteral("DirectChatsActive"));

    connect(this, &RoomManager::currentRoomChanged, this, [this]() {
        m_timelineModel->setRoom(m_currentRoom);
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

TimelineModel *RoomManager::timelineModel() const
{
    return m_timelineModel;
}

MessageFilterModel *RoomManager::messageFilterModel() const
{
    return m_messageFilterModel;
}

MediaMessageFilterModel *RoomManager::mediaMessageFilterModel() const
{
    return m_mediaMessageFilterModel;
}

UriResolveResult RoomManager::resolveResource(const Uri &uri)
{
    return UriResolverBase::visitResource(m_connection, uri);
}

void RoomManager::resolveResource(const QString &idOrUri, const QString &action)
{
    Uri uri{idOrUri};
    if (!uri.isValid()) {
        Q_EMIT warning(i18n("Malformed or empty Matrix id"), i18n("%1 is not a correct Matrix identifier", idOrUri));
        return;
    }

    if (uri.type() != Uri::NonMatrix) {
        if (!m_connection) {
            return;
        }
        if (!action.isEmpty()) {
            uri.setAction(action);
        }
        // TODO we should allow the user to select a connection.
    }

    const auto result = visitResource(m_connection, uri);
    if (result == Quotient::CouldNotResolve) {
        Q_EMIT warning(i18n("Room not found"), i18n("There's no room %1 in the room list. Check the spelling and the account.", idOrUri));
    } else { // Invalid cases should have been eliminated earlier
        Q_ASSERT(result == Quotient::UriResolved);

        if (uri.type() == Uri::RoomAlias || uri.type() == Uri::RoomId) {
            connectSingleShot(m_connection, &Connection::newRoom, this, [this, uri](Room *room) {
                resolveResource(room->id());
            });
        }
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

void RoomManager::viewEventMenu(const QString &eventId, NeoChatRoom *room, const QString &selectedText)
{
    const auto &event = **room->findInTimeline(eventId);
    const auto eventHandler = EventHandler(room, &event);

    if (eventHandler.getMediaInfo().contains("mimeType"_ls)) {
        Q_EMIT showFileMenu(eventId,
                            eventHandler.getAuthor(),
                            eventHandler.messageComponentType(),
                            eventHandler.getPlainBody(),
                            eventHandler.getMediaInfo()["mimeType"_ls].toString(),
                            room->fileTransferInfo(eventId));
        return;
    }

    Q_EMIT showMessageMenu(eventId,
                           eventHandler.getAuthor(),
                           eventHandler.messageComponentType(),
                           eventHandler.getPlainBody(),
                           eventHandler.getRichBody(),
                           selectedText);
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
    Q_ASSERT(m_connection);

    if (!m_arg.isEmpty()) {
        resolveResource(m_arg);
    }

    if (m_currentRoom) {
        // we opened a room with the arg parsing already
        return;
    }

    openRoomForActiveConnection();

    connect(this, &RoomManager::connectionChanged, this, &RoomManager::openRoomForActiveConnection);
}

QString RoomManager::lastSpaceId() const
{
    if (!m_connection) {
        return {};
    }
    return m_lastSpaceConfig.readEntry(m_connection->userId(), QString());
}

void RoomManager::setLastSpaceId(const QString &lastSpaceId)
{
    if (!m_connection) {
        return;
    }

    const auto currentLastSpaceId = m_lastSpaceConfig.readEntry(m_connection->userId(), QString());
    if (lastSpaceId == currentLastSpaceId) {
        return;
    }
    m_lastSpaceConfig.writeEntry(m_connection->userId(), lastSpaceId);
    Q_EMIT lastSpaceIdChanged();
}

bool RoomManager::directChatsActive() const
{
    if (!m_connection) {
        return {};
    }
    return m_directChatsConfig.readEntry(m_connection->userId(), bool());
}

void RoomManager::setDirectChatsActive(bool directChatsActive)
{
    if (!m_connection) {
        return;
    }

    const auto currentDirectChatsActive = m_directChatsConfig.readEntry(m_connection->userId(), bool());
    if (directChatsActive == currentDirectChatsActive) {
        return;
    }
    m_directChatsConfig.writeEntry(m_connection->userId(), directChatsActive);
    Q_EMIT directChatsActiveChanged();
}

void RoomManager::openRoomForActiveConnection()
{
    if (!m_connection) {
        return;
    }
    // Read from last open room
    QString roomId = m_lastRoomConfig.readEntry(m_connection->userId(), QString());

    // TODO remove legacy check at some point.
    if (roomId.isEmpty()) {
        roomId = NeoChatConfig::self()->openRoom();
    }

    if (!roomId.isEmpty()) {
        // Here we can cast because the controller has been configured to
        // return NeoChatRoom instead of simple Quotient::Room
        const auto room = qobject_cast<NeoChatRoom *>(m_connection->room(roomId));

        if (room) {
            resolveResource(room->id());
        }
    }
}

void RoomManager::openWindow(NeoChatRoom *room)
{
    // forward the call to QML
    Q_EMIT openRoomInNewWindow(room);
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

    if (m_currentRoom && !m_currentRoom->editCache()->editId().isEmpty()) {
        m_currentRoom->editCache()->setEditId({});
    }
    if (m_currentRoom && !m_currentRoom->isSpace() && m_chatDocumentHandler) {
        // We're doing these things here because it is critical that they are switched at the same time
        if (m_chatDocumentHandler->document()) {
            m_currentRoom->mainCache()->setSavedText(m_chatDocumentHandler->document()->textDocument()->toPlainText());
            m_chatDocumentHandler->setRoom(neoChatRoom);
            m_chatDocumentHandler->document()->textDocument()->setPlainText(neoChatRoom->mainCache()->savedText());
            neoChatRoom->mainCache()->setText(neoChatRoom->mainCache()->savedText());
        } else {
            m_chatDocumentHandler->setRoom(neoChatRoom);
        }
    }

    if (m_currentRoom) {
        if (m_currentRoom->id() == room->id()) {
            Q_EMIT goToEvent(eventId);
        } else {
            m_lastCurrentRoom = std::exchange(m_currentRoom, neoChatRoom);
            Q_EMIT currentRoomChanged();

            if (neoChatRoom->isSpace()) {
                m_lastSpaceConfig.writeEntry(m_connection->userId(), room->id());
                Q_EMIT replaceSpaceHome(neoChatRoom);
            } else {
                Q_EMIT replaceRoom(neoChatRoom, eventId);
            }
        }
    } else {
        m_lastCurrentRoom = std::exchange(m_currentRoom, neoChatRoom);
        Q_EMIT currentRoomChanged();
        if (neoChatRoom->isSpace()) {
            m_lastSpaceConfig.writeEntry(m_connection->userId(), room->id());
            Q_EMIT pushSpaceHome(neoChatRoom);
        } else {
            Q_EMIT pushRoom(neoChatRoom, eventId);
        }
    }

    // Save last open room
    m_lastRoomConfig.writeEntry(m_connection->userId(), room->id());
}

void RoomManager::joinRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QStringList &viaServers)
{
    auto job = account->joinRoom(roomAliasOrId, viaServers);
    connectSingleShot(job, &Quotient::BaseJob::finished, this, [this, account](Quotient::BaseJob *finish) {
        if (finish->status() == Quotient::BaseJob::Success) {
            connectSingleShot(account, &Quotient::Connection::newRoom, this, [this](Quotient::Room *room) {
                resolveResource(room->id());
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

NeoChatConnection *RoomManager::connection() const
{
    return m_connection;
}

void RoomManager::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
}

#include "moc_roommanager.cpp"
