// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2021 Alexey Rusakov <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roommanager.h"

#include "chatbarcache.h"
#include "controller.h"
#include "eventhandler.h"
#include "models/timelinemodel.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "spacehierarchycache.h"

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

    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this](NeoChatConnection *connection) {
        setConnection(connection);
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
        if (!action.isEmpty() && (uri.type() != Uri::UserId || action != "join"_ls)) {
            uri.setAction(action);
        }
        // TODO we should allow the user to select a connection.
    }

    const auto result = visitResource(m_connection, uri);
    if (result == Quotient::CouldNotResolve) {
        if ((uri.type() == Uri::RoomAlias || uri.type() == Uri::RoomId) && action != "no_join"_ls) {
            Q_EMIT askJoinRoom(uri.primaryId());
        }
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

void RoomManager::openRoomForActiveConnection()
{
    if (!m_connection) {
        m_currentRoom = nullptr;
    }
    if (m_lastRoomConfig.readEntry(m_connection->userId(), QString()).isEmpty()) {
        setCurrentRoom({});
    } else {
        resolveResource(m_lastRoomConfig.readEntry(m_connection->userId(), QString()));
    }
    setCurrentSpace(m_lastSpaceConfig.readEntry(m_connection->userId(), QString()), false);
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

void RoomManager::visitRoom(Room *r, const QString &eventId)
{
    auto room = qobject_cast<NeoChatRoom *>(r);

    if (m_currentRoom && !m_currentRoom->editCache()->editId().isEmpty()) {
        m_currentRoom->editCache()->setEditId({});
    }
    if (m_currentRoom && !m_currentRoom->isSpace() && m_chatDocumentHandler) {
        // We're doing these things here because it is critical that they are switched at the same time
        if (m_chatDocumentHandler->document()) {
            m_currentRoom->mainCache()->setSavedText(m_chatDocumentHandler->document()->textDocument()->toPlainText());
            m_chatDocumentHandler->setRoom(room);
            if (room) {
                m_chatDocumentHandler->document()->textDocument()->setPlainText(room->mainCache()->savedText());
                room->mainCache()->setText(room->mainCache()->savedText());
            }
        } else {
            m_chatDocumentHandler->setRoom(room);
        }
    }

    if (!room) {
        setCurrentRoom({});
        return;
    }

    if (m_currentRoom && m_currentRoom->id() == room->id()) {
        Q_EMIT goToEvent(eventId);
    } else {
        setCurrentRoom(room->id());
    }
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
    Q_EMIT externalUrl(url);
    return true;
}

void RoomManager::leaveRoom(NeoChatRoom *room)
{
    if (!room) {
        return;
    }

    if (m_currentRoom && m_currentRoom->id() == room->id()) {
        setCurrentRoom({});
    }

    if (m_currentSpaceId == room->id()) {
        setCurrentSpace({});
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

void RoomManager::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
}

void RoomManager::setCurrentSpace(const QString &spaceId, bool setRoom)
{
    m_currentSpaceId = spaceId;
    Q_EMIT currentSpaceChanged();
    m_lastSpaceConfig.writeEntry(m_connection->userId(), spaceId);

    if (!setRoom) {
        return;
    }
    if (spaceId.length() > 3) {
        resolveResource(spaceId, "no_join"_ls);
    } else {
        visitRoom({}, {});
    }
}

void RoomManager::setCurrentRoom(const QString &roomId)
{
    if (roomId.isEmpty()) {
        m_currentRoom = nullptr;
    } else {
        m_currentRoom = dynamic_cast<NeoChatRoom *>(m_connection->room(roomId));
    }
    Q_EMIT currentRoomChanged();
    if (m_connection) {
        m_lastRoomConfig.writeEntry(m_connection->userId(), roomId);
    }
    if (roomId.isEmpty()) {
        return;
    }
    if (m_currentRoom->isSpace()) {
        return;
    }
    if (m_currentRoom->isDirectChat() && m_currentSpaceId != "DM"_ls) {
        setCurrentSpace("DM"_ls, false);
        return;
    }
    const auto &parentSpaces = SpaceHierarchyCache::instance().parentSpaces(roomId);
    if (parentSpaces.contains(m_currentSpaceId)) {
        return;
    }
    if (const auto &parent = m_connection->room(m_currentRoom->canonicalParent())) {
        for (const auto &parentParent : SpaceHierarchyCache::instance().parentSpaces(parent->id())) {
            if (SpaceHierarchyCache::instance().parentSpaces(parentParent).isEmpty()) {
                setCurrentSpace(parentParent, false);
                return;
            }
        }
        setCurrentSpace(parent->id(), false);
        return;
    }
    for (const auto &space : parentSpaces) {
        if (SpaceHierarchyCache::instance().parentSpaces(space).isEmpty()) {
            setCurrentSpace(space, false);
            return;
        }
    }
    setCurrentSpace({}, false);
}

QString RoomManager::currentSpace() const
{
    return m_currentSpaceId;
}

#include "moc_roommanager.cpp"
