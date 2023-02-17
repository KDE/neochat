// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2021 Alexey Rusakov <TODO>
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roommanager.h"
#include "controller.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include <KLocalizedString>
#include <QDesktopServices>
#include <QQuickTextDocument>
#include <QStandardPaths>
#include <csapi/joining.h>
#ifdef QUOTIENT_07
#include <csapi/knocking.h>
#endif
#include <qt_connection_util.h>
#include <user.h>

#ifndef Q_OS_ANDROID
#include <KIO/OpenUrlJob>
#endif

RoomManager::RoomManager(QObject *parent)
    : QObject(parent)
    , m_currentRoom(nullptr)
    , m_lastCurrentRoom(nullptr)
    , m_config(KConfig("data", KConfig::SimpleConfig, QStandardPaths::AppDataLocation))
{
    m_lastRoomConfig = m_config.group("LastOpenRoom");
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
            enterRoom(room);
        }
    } else {
        Q_EMIT pushWelcomePage();
    }
}

void RoomManager::enterRoom(NeoChatRoom *room)
{
    if (m_currentRoom && !m_currentRoom->chatBoxEditId().isEmpty()) {
        m_currentRoom->setChatBoxEditId("");
    }
    if (m_currentRoom && m_chatDocumentHandler) {
        // We're doing these things here because it is critical that they are switched at the same time
        m_currentRoom->setSavedText(m_chatDocumentHandler->document()->textDocument()->toPlainText());
        m_chatDocumentHandler->setRoom(room);
        m_chatDocumentHandler->document()->textDocument()->setPlainText(room->savedText());
    }
    m_lastCurrentRoom = std::exchange(m_currentRoom, room);
    Q_EMIT currentRoomChanged();

    if (!m_lastCurrentRoom) {
        Q_EMIT pushRoom(room, QString());
    } else {
        Q_EMIT replaceRoom(m_currentRoom, QString());
    }

    // Save last open room
    m_lastRoomConfig.writeEntry(Controller::instance().activeConnection()->userId(), room->id());
}

void RoomManager::openWindow(NeoChatRoom *room)
{
    // forward the call to QML
    Q_EMIT openRoomInNewWindow(room);
}

UriResolveResult RoomManager::visitUser(User *user, const QString &action)
{
    if (action == "mention" || action.isEmpty()) {
        // send it has QVariantMap because the properties in the
#ifdef QUOTIENT_07
        user->load();
#endif
        Q_EMIT showUserDetail(user);
    } else if (action == "_interactive") {
        user->requestDirectChat();
    } else if (action == "chat") {
#ifdef QUOTIENT_07
        user->load();
#endif
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
    auto job = account->joinRoom(QUrl::toPercentEncoding(roomAliasOrId), viaServers);
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

// TODO: maybe need use the function upstream later
#ifdef QUOTIENT_07
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
#endif

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
