// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2021 Alexey Rusakov <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roommanager.h"

#include "chatbarcache.h"
#include "contentprovider.h"
#include "controller.h"
#include "eventhandler.h"
#include "models/actionsmodel.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "spacehierarchycache.h"
#include "urlhelper.h"

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
    , m_roomListModel(new RoomListModel(this))
    , m_sortFilterRoomListModel(new SortFilterRoomListModel(m_roomListModel, this))
    , m_sortFilterSpaceListModel(new SortFilterSpaceListModel(m_roomListModel, this))
    , m_roomTreeModel(new RoomTreeModel(this))
    , m_sortFilterRoomTreeModel(new SortFilterRoomTreeModel(m_roomTreeModel, this))
    , m_timelineModel(new TimelineModel(this))
    , m_messageFilterModel(new MessageFilterModel(this, m_timelineModel))
    , m_mediaMessageFilterModel(new MediaMessageFilterModel(this, m_messageFilterModel))
    , m_userListModel(new UserListModel(this))
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(UBUNTU_TOUCH)
    m_isMobile = true;
#else
    // Mostly for debug purposes and for platforms which are always mobile,
    // such as Plasma Mobile
    if (qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_MOBILE")) {
        m_isMobile = QByteArrayList{"1", "true"}.contains(qgetenv("QT_QUICK_CONTROLS_MOBILE"));
    }
#endif

    connect(this, &RoomManager::currentRoomChanged, this, [this]() {
        m_userListModel->setRoom(m_currentRoom);
        m_timelineModel->setRoom(m_currentRoom);
        m_sortFilterRoomTreeModel->setCurrentRoom(m_currentRoom);
    });

    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this](NeoChatConnection *connection) {
        setConnection(connection);
    });
    connect(this, &RoomManager::connectionChanged, this, [this]() {
        m_roomListModel->setConnection(m_connection);
        m_roomTreeModel->setConnection(m_connection);
    });
    connect(m_sortFilterSpaceListModel, &SortFilterSpaceListModel::layoutChanged, m_sortFilterRoomTreeModel, &SortFilterRoomTreeModel::invalidate);
    connect(&ActionsModel::instance(), &ActionsModel::resolveResource, this, [this](const QString &idOrUri, const QString &action) {
        resolveResource(idOrUri, action);
    });
    connect(&ActionsModel::instance(), &ActionsModel::knockRoom, this, &RoomManager::knockRoom);
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowStateEventChanged, this, [this] {
        if (m_messageFilterModel) {
            m_messageFilterModel->invalidate();
        }
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLeaveJoinEventChanged, this, [this] {
        if (m_messageFilterModel) {
            m_messageFilterModel->invalidate();
        }
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowRenameChanged, this, [this] {
        if (m_messageFilterModel) {
            m_messageFilterModel->invalidate();
        }
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowAvatarUpdateChanged, this, [this] {
        if (m_messageFilterModel) {
            m_messageFilterModel->invalidate();
        }
    });
    connect(m_timelineModel->timelineMessageModel(), &MessageModel::modelResetComplete, this, &RoomManager::activateUserModel);
    MessageFilterModel::setShowAllEvents(NeoChatConfig::self()->showAllEvents());
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowAllEventsChanged, this, [this] {
        MessageFilterModel::setShowAllEvents(NeoChatConfig::self()->showAllEvents());
        if (m_messageFilterModel) {
            m_messageFilterModel->invalidate();
        }
    });
    MessageFilterModel::setShowDeletedMessages(NeoChatConfig::self()->showDeletedMessages());
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowDeletedMessagesChanged, this, [this] {
        MessageFilterModel::setShowDeletedMessages(NeoChatConfig::self()->showDeletedMessages());
        if (m_messageFilterModel) {
            m_messageFilterModel->invalidate();
        }
    });
    ContentProvider::self().setThreadsEnabled(NeoChatConfig::threads());
    MessageModel::setThreadsEnabled(NeoChatConfig::threads());
    connect(NeoChatConfig::self(), &NeoChatConfig::ThreadsChanged, this, [this] {
        ContentProvider::self().setThreadsEnabled(NeoChatConfig::threads());
        MessageModel::setThreadsEnabled(NeoChatConfig::threads());
        if (m_timelineModel) {
            Q_EMIT m_timelineModel->threadsEnabledChanged();
        }
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::SortOrderChanged, this, [this]() {
        m_sortFilterRoomTreeModel->invalidate();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::CollapsedChanged, this, [this]() {
        m_sortFilterRoomTreeModel->invalidate();
    });
    SortFilterRoomTreeModel::setShowAllRoomsInHome(NeoChatConfig::allRoomsInHome());
    connect(NeoChatConfig::self(), &NeoChatConfig::AllRoomsInHomeChanged, this, [this]() {
        SortFilterRoomTreeModel::setShowAllRoomsInHome(NeoChatConfig::allRoomsInHome());
        m_sortFilterRoomTreeModel->invalidate();
        if (NeoChatConfig::allRoomsInHome()) {
            resetState();
        }
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

RoomListModel *RoomManager::roomListModel() const
{
    return m_roomListModel;
}

SortFilterRoomListModel *RoomManager::sortFilterRoomListModel() const
{
    return m_sortFilterRoomListModel;
}

SortFilterSpaceListModel *RoomManager::sortFilterSpaceListModel() const
{
    return m_sortFilterSpaceListModel;
}

RoomTreeModel *RoomManager::roomTreeModel() const
{
    return m_roomTreeModel;
}

SortFilterRoomTreeModel *RoomManager::sortFilterRoomTreeModel() const
{
    return m_sortFilterRoomTreeModel;
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

UserListModel *RoomManager::userListModel() const
{
    return m_userListModel;
}

void RoomManager::activateUserModel()
{
    m_userListModel->activate();
}

void RoomManager::resolveResource(const QString &idOrUri, const QString &action)
{
    resolveResource(Uri{idOrUri}, action);
}

void RoomManager::resolveResource(Uri uri, const QString &action)
{
    if (!uri.isValid()) {
        Q_EMIT showMessage(MessageType::Warning, i18n("Malformed or empty Matrix id<br />%1 is not a correct Matrix identifier", uri.toDisplayString()));
        return;
    }

    if (action == "qr"_L1) {
        if (uri.type() == Uri::NonMatrix) {
            Q_EMIT externalUrl(uri.toUrl());
            return;
        }
        if (uri.type() != Uri::UserId) {
            uri.setAction(QStringLiteral("join"));
        } else {
            uri.setAction(action);
        }
    }

    // For matrix URIs:
    if (uri.type() != Uri::NonMatrix) {
        if (!m_connection) {
            return;
        }
        // Once a join is confirmed, set the action to "join" so it skips the confirmation check.
        if (action == "join_confirmed"_L1) {
            uri.setAction(QStringLiteral("join"));
        }
        // TODO we should allow the user to select a connection.
    }

    const auto result = visitResource(m_connection, uri);

    // If we are not already in the room:
    if (result == Quotient::CouldNotResolve) {
        if ((uri.type() == Uri::RoomAlias || uri.type() == Uri::RoomId) && action == "join"_L1) {
            Q_EMIT askJoinRoom(uri.primaryId());
        }
    }
}

void RoomManager::maximizeMedia(const QString &eventId)
{
    if (eventId.isEmpty()) {
        qWarning() << "Tried to open media for empty event id";
        return;
    }

    const auto index = m_mediaMessageFilterModel->getRowForEventId(eventId);
    if (index == -1) {
        qWarning() << "Tried to open media for unknown event id" << eventId;
        return;
    }

    Q_EMIT showMaximizedMedia(index);
}

void RoomManager::maximizeCode(NeochatRoomMember *author, const QDateTime &time, const QString &codeText, const QString &language)
{
    if (codeText.isEmpty()) {
        return;
    }
    Q_EMIT showMaximizedCode(author, time, codeText, language);
}

void RoomManager::requestFullScreenClose()
{
    Q_EMIT closeFullScreen();
}

void RoomManager::viewEventSource(const QString &eventId)
{
    Q_EMIT showEventSource(eventId);
}

void RoomManager::viewEventMenu(const QString &eventId, NeoChatRoom *room, const QString &selectedText, const QString &hoveredLink)
{
    if (eventId.isEmpty()) {
        qWarning() << "Tried to open event menu with empty event id";
        return;
    }

    const auto it = room->findInTimeline(eventId);
    if (it == room->historyEdge()) {
        // This is probably a pending event
        return;
    }
    const auto &event = **it;
    Q_EMIT showDelegateMenu(eventId,
                            room->qmlSafeMember(event.senderId()),
                            MessageComponentType::typeForEvent(event),
                            EventHandler::plainBody(room, &event),
                            EventHandler::richBody(room, &event),
                            EventHandler::mediaInfo(room, &event)["mimeType"_L1].toString(),
                            room->fileTransferInfo(eventId),
                            selectedText,
                            hoveredLink);
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

    if (m_isMobile) {
        QString lastSpace = m_lastRoomConfig.readEntry(u"lastSpace"_s, QString());
        // We can't have empty keys in KConfig, so we stored it as "Home"
        if (lastSpace == u"Home"_s) {
            lastSpace.clear();
        }
        setCurrentSpace(lastSpace, false);
        // We don't want to open a room on startup on mobile
        return;
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
        setCurrentRoom({});
        setCurrentSpace({}, false);
        return;
    }
    auto lastSpace = m_lastRoomConfig.readEntry(u"lastSpace"_s, QString());
    if (lastSpace == u"Home"_s) {
        lastSpace.clear();
    }
    setCurrentSpace(lastSpace, true);
}

UriResolveResult RoomManager::visitUser(User *user, const QString &action)
{
    if (action == "mention"_L1 || action == "qr"_L1 || action.isEmpty()) {
        user->load();
        Q_EMIT showUserDetail(user, action == "qr"_L1 ? nullptr : currentRoom());
    } else if (action == "_interactive"_L1) {
        user->requestDirectChat();
    } else if (action == "chat"_L1) {
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

    if (!room) {
        setCurrentRoom({});
        return;
    }

    // It's important that we compare room *objects* here, not just room *ids*, since we need to deal with the object changing when going invite -> joined
    if (m_currentRoom && m_currentRoom == room) {
        if (!eventId.isEmpty()) {
            Q_EMIT goToEvent(eventId);
        }
    } else {
        setCurrentRoom(room->id());
    }
}

void RoomManager::joinRoom(Quotient::Connection *account, const QString &roomAliasOrId, const QStringList &viaServers)
{
    QStringList vias = viaServers;

    // If no one gives us a homeserver suggestion, try the server specified in the alias/id.
    // Otherwise joining a remote room not on our homeserver will fail.
    // This is a hack and we're not supposed to do it. With room ids not containing the server going forward, it won't work anymore for new room versions.
    // FIXME: Let's keep it around anyway for now, remove it at some point, though
    if (vias.empty() && roomAliasOrId.contains(':'_L1)) {
        vias.append(roomAliasOrId.mid(roomAliasOrId.lastIndexOf(':'_L1) + 1));
    }

    auto job = account->joinRoom(roomAliasOrId, vias);
    connect(
        job.get(),
        &Quotient::BaseJob::finished,
        this,
        [this, account, roomAliasOrId](Quotient::BaseJob *finish) {
            if (finish->status() == Quotient::BaseJob::Success) {
                connect(
                    account,
                    &NeoChatConnection::newRoom,
                    this,
                    [this](Quotient::Room *room) {
                        resolveResource(room->id());
                    },
                    Qt::SingleShotConnection);
            } else {
                Q_EMIT showMessage(MessageType::Warning, i18n("Failed to join %1:<br />%2", roomAliasOrId, finish->errorString()));
            }
        },
        Qt::SingleShotConnection);
}

void RoomManager::knockRoom(NeoChatConnection *account, const QString &roomAliasOrId, const QString &reason, const QStringList &viaServers)
{
    auto job = account->callApi<KnockRoomJob>(roomAliasOrId, viaServers, viaServers, reason);
    // Upon completion, ensure a room object is created in case it hasn't come
    // with a sync yet. If the room object is not there, provideRoom() will
    // create it in Join state. finished() is used here instead of success()
    // to overtake clients that may add their own slots to finished().
    connect(
        job.get(),
        &BaseJob::finished,
        this,
        [this, job, account] {
            if (job->status() == Quotient::BaseJob::Success) {
                connect(
                    account,
                    &NeoChatConnection::newRoom,
                    this,
                    [this](Quotient::Room *room) {
                        Q_EMIT showMessage(MessageType::Information, i18n("You requested to join '%1'", room->name()));
                    },
                    Qt::SingleShotConnection);
            } else {
                Q_EMIT showMessage(MessageType::Warning, i18n("Failed to request joining room<br />%1", job->errorString()));
            }
        },
        Qt::SingleShotConnection);
}

void RoomManager::roomLeft(const QString &id)
{
    if (id.isEmpty()) {
        return;
    }

    if (m_currentRoom && m_currentRoom->id() == id) {
        setCurrentRoom({});
    }

    if (m_currentSpaceId == id) {
        setCurrentSpace({});
    }
}

bool RoomManager::visitNonMatrix(const QUrl &url)
{
    UrlHelper().openUrl(url);
    return true;
}

void RoomManager::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }

    if (m_connection != nullptr) {
        m_connection->disconnect(this);
    }

    m_connection = connection;

    if (m_connection != nullptr) {
        m_lastRoomConfig = m_config->group(m_connection->userId()).group(u"LastOpenRoom"_s);
        connect(m_connection, &NeoChatConnection::showMessage, this, &RoomManager::showMessage);
        connect(m_connection, &NeoChatConnection::createdRoom, this, [this](Quotient::Room *room) {
            resolveResource(room->id());
        });
        connect(m_connection, &NeoChatConnection::directChatAvailable, this, [this](Quotient::Room *directChat) {
            resolveResource(directChat->id());
        });
        connect(m_connection, &NeoChatConnection::joinedRoom, this, [this](const auto &room, const auto &previousRoom) {
            if (m_currentRoom == previousRoom) {
                resolveResource(room->id());
            }
        });
    }

    Q_EMIT connectionChanged();
}

void RoomManager::setCurrentSpace(const QString &spaceId, bool setRoom)
{
    m_currentSpaceId = spaceId;

    // This need to happen before the signal so TreeView.expandRecursively() can work nicely.
    m_sortFilterRoomTreeModel->setActiveSpaceId(m_currentSpaceId);
    m_sortFilterRoomTreeModel->setMode(m_currentSpaceId == u"DM"_s ? SortFilterRoomTreeModel::DirectChats : SortFilterRoomTreeModel::Rooms);

    Q_EMIT currentSpaceChanged();
    if (m_connection) {
        m_lastRoomConfig.writeEntry(u"lastSpace"_s, spaceId.isEmpty() ? u"Home"_s : spaceId);
    }

    if (!setRoom) {
        return;
    }

    // We intentionally don't want to open the last room on mobile
    if (m_isMobile) {
        return;
    }

    // We can't have empty keys in KConfig, so it's stored as "Home"
    if (const auto &lastRoom = m_lastRoomConfig.readEntry(spaceId.isEmpty() ? u"Home"_s : spaceId, QString()); !lastRoom.isEmpty()) {
        resolveResource(lastRoom, "no_join"_L1);
        return;
    }
    if (!spaceId.isEmpty() && spaceId != u"DM"_s) {
        resolveResource(spaceId, "no_join"_L1);
        return;
    }
    setCurrentRoom({});
}

void RoomManager::setCurrentRoom(const QString &roomId)
{
    if (m_currentRoom != nullptr) {
        m_currentRoom->disconnect(this);
    }

    if (roomId.isEmpty()) {
        m_currentRoom = nullptr;
    } else {
        m_currentRoom = dynamic_cast<NeoChatRoom *>(m_connection->room(roomId));
    }

    if (m_currentRoom != nullptr) {
        connect(m_currentRoom, &NeoChatRoom::showMessage, this, &RoomManager::showMessage);
    }

    Q_EMIT currentRoomChanged();
    if (m_connection) {
        if (roomId.isEmpty()) {
            m_lastRoomConfig.deleteEntry(m_currentSpaceId);
        } else {
            // We can't have empty keys in KConfig, so name it "Home"
            if (m_currentSpaceId.isEmpty()) {
                m_lastRoomConfig.writeEntry(u"Home"_s, roomId);
            } else {
                m_lastRoomConfig.writeEntry(m_currentSpaceId, roomId);
            }
        }
    }
    if (roomId.isEmpty()) {
        return;
    }
    if (m_currentRoom->isSpace()) {
        return;
    }
    if (m_currentRoom->isDirectChat()) {
        if (m_currentSpaceId != "DM"_L1) {
            setCurrentSpace("DM"_L1, false);
        }
        return;
    }
    const auto &parentSpaces = SpaceHierarchyCache::instance().parentSpaces(roomId);
    if (parentSpaces.contains(m_currentSpaceId)) {
        return;
    }
    static auto config = NeoChatConfig::self();
    if (config->allRoomsInHome()) {
        setCurrentSpace({}, false);
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

void RoomManager::clearCurrentRoom()
{
    setCurrentRoom(QString());
}

QString RoomManager::currentSpace() const
{
    return m_currentSpaceId;
}

void RoomManager::resetState()
{
    setCurrentRoom({});
    setCurrentSpace({});
}

#include "moc_roommanager.cpp"
