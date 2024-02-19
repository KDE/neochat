// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "roomlistmodel.h"

#include "eventhandler.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "spacehierarchycache.h"

#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#ifndef Q_OS_ANDROID
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#endif
#endif

#include <KLocalizedString>
#include <QGuiApplication>

using namespace Quotient;

Q_DECLARE_METATYPE(Quotient::JoinState)

RoomListModel::RoomListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &RoomListModel::highlightCountChanged, this, [this]() {
#if QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#ifndef Q_OS_ANDROID
        // copied from Telegram desktop
        const auto launcherUrl = "application://org.kde.neochat.desktop"_ls;
        // Gnome requires that count is a 64bit integer
        const qint64 counterSlice = std::min(m_highlightCount, 9999);
        QVariantMap dbusUnityProperties;

        if (counterSlice > 0) {
            dbusUnityProperties["count"_ls] = counterSlice;
            dbusUnityProperties["count-visible"_ls] = true;
        } else {
            dbusUnityProperties["count-visible"_ls] = false;
        }

        auto signal = QDBusMessage::createSignal("/com/canonical/unity/launcherentry/neochat"_ls, "com.canonical.Unity.LauncherEntry"_ls, "Update"_ls);

        signal.setArguments({launcherUrl, dbusUnityProperties});

        QDBusConnection::sessionBus().send(signal);
#endif // Q_OS_ANDROID
#else
        qGuiApp->setBadgeNumber(m_highlightCount);
#endif // QT_VERSION_CHECK(6, 6, 0)
    });
    connect(&SpaceHierarchyCache::instance(), &SpaceHierarchyCache::spaceHierarchyChanged, this, [this]() {
        Q_EMIT dataChanged(index(0, 0), index(rowCount(), 0), {IsChildSpaceRole});
    });
}

RoomListModel::~RoomListModel() = default;

Quotient::Connection *RoomListModel::connection() const
{
    return m_connection;
}

void RoomListModel::setConnection(Connection *connection)
{
    if (connection == m_connection) {
        return;
    }
    if (m_connection) {
        m_connection->disconnect(this);
    }
    if (!connection) {
        qDebug() << "Removing current connection...";
        m_connection = nullptr;
        beginResetModel();
        m_rooms.clear();
        endResetModel();
        return;
    }

    m_connection = connection;

    for (NeoChatRoom *room : std::as_const(m_rooms)) {
        room->disconnect(this);
    }

    connect(connection, &Connection::connected, this, &RoomListModel::doResetModel);
    connect(connection, &Connection::invitedRoom, this, &RoomListModel::updateRoom);
    connect(connection, &Connection::joinedRoom, this, &RoomListModel::updateRoom);
    connect(connection, &Connection::leftRoom, this, &RoomListModel::updateRoom);
    connect(connection, &Connection::aboutToDeleteRoom, this, &RoomListModel::deleteRoom);
    connect(connection, &Connection::directChatsListChanged, this, [this, connection](Quotient::DirectChatsMap additions, Quotient::DirectChatsMap removals) {
        auto refreshRooms = [this, &connection](Quotient::DirectChatsMap rooms) {
            for (const QString &roomID : std::as_const(rooms)) {
                auto room = connection->room(roomID);
                if (room) {
                    refresh(static_cast<NeoChatRoom *>(room));
                }
            }
        };

        refreshRooms(std::move(additions));
        refreshRooms(std::move(removals));
    });

    doResetModel();

    Q_EMIT connectionChanged();
}

void RoomListModel::doResetModel()
{
    beginResetModel();
    m_rooms.clear();
    const auto rooms = m_connection->allRooms();
    for (const auto &room : rooms) {
        doAddRoom(room);
    }
    endResetModel();
    refreshNotificationCount();
}

NeoChatRoom *RoomListModel::roomAt(int row) const
{
    return m_rooms.at(row);
}

void RoomListModel::doAddRoom(Room *r)
{
    if (auto room = static_cast<NeoChatRoom *>(r)) {
        m_rooms.append(room);
        connectRoomSignals(room);
        Q_EMIT roomAdded(room);
    } else {
        qCritical() << "Attempt to add nullptr to the room list";
        Q_ASSERT(false);
    }
}

void RoomListModel::connectRoomSignals(NeoChatRoom *room)
{
    connect(room, &Room::displaynameChanged, this, [this, room] {
        refresh(room, {DisplayNameRole});
    });
    connect(room, &Room::unreadStatsChanged, this, [this, room] {
        refresh(room, {NotificationCountRole, HighlightCountRole});
    });
    connect(room, &Room::notificationCountChanged, this, [this, room] {
        refresh(room);
    });
    connect(room, &Room::highlightCountChanged, this, [this, room] {
        refresh(room);
    });
    connect(room, &Room::avatarChanged, this, [this, room] {
        refresh(room, {AvatarRole});
    });
    connect(room, &Room::tagsChanged, this, [this, room] {
        refresh(room);
    });
    connect(room, &Room::joinStateChanged, this, [this, room] {
        refresh(room);
    });
    connect(room, &Room::addedMessages, this, [this, room] {
        refresh(room, {SubtitleTextRole, LastActiveTimeRole});
    });
    connect(room, &Room::pendingEventMerged, this, [this, room] {
        refresh(room, {SubtitleTextRole});
    });
    connect(room, &Room::unreadStatsChanged, this, &RoomListModel::refreshNotificationCount);
    connect(room, &Room::highlightCountChanged, this, &RoomListModel::refreshHighlightCount);
}

int RoomListModel::notificationCount() const
{
    return m_notificationCount;
}

int RoomListModel::highlightCount() const
{
    return m_highlightCount;
}

void RoomListModel::refreshNotificationCount()
{
    int count = 0;
    for (auto room : std::as_const(m_rooms)) {
        count += room->notificationCount();
    }
    if (m_notificationCount == count) {
        return;
    }
    m_notificationCount = count;
    Q_EMIT notificationCountChanged();
}

void RoomListModel::refreshHighlightCount()
{
    int count = 0;
    for (auto room : std::as_const(m_rooms)) {
        count += room->highlightCount();
    }
    if (m_highlightCount == count) {
        return;
    }
    m_highlightCount = count;
    Q_EMIT highlightCountChanged();
}

void RoomListModel::updateRoom(Room *room, Room *prev)
{
    // There are two cases when this method is called:
    // 1. (prev == nullptr) adding a new room to the room list
    // 2. (prev != nullptr) accepting/rejecting an invitation or inviting to
    //    the previously left room (in both cases prev has the previous state).
    if (prev == room) {
        qCritical() << "RoomListModel::updateRoom: room tried to replace itself";
        refresh(static_cast<NeoChatRoom *>(room));
        return;
    }
    if (prev && room->id() != prev->id()) {
        qCritical() << "RoomListModel::updateRoom: attempt to update room" << room->id() << "to" << prev->id();
        // That doesn't look right but technically we still can do it.
    }
    // Ok, we're through with pre-checks, now for the real thing.
    auto newRoom = static_cast<NeoChatRoom *>(room);
    const auto it = std::find_if(m_rooms.begin(), m_rooms.end(), [prev, newRoom](const NeoChatRoom *r) {
        return r == prev || r == newRoom;
    });
    if (it != m_rooms.end()) {
        const int row = it - m_rooms.begin();
        // There's no guarantee that prev != newRoom
        if (*it == prev && *it != newRoom) {
            prev->disconnect(this);
            m_rooms.replace(row, newRoom);
            connectRoomSignals(newRoom);
        }
        Q_EMIT dataChanged(index(row), index(row));
    } else {
        beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
        doAddRoom(newRoom);
        endInsertRows();
    }
}

void RoomListModel::deleteRoom(Room *room)
{
    qDebug() << "Deleting room" << room->id();
    const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
    if (it == m_rooms.end()) {
        return; // Already deleted, nothing to do
    }
    qDebug() << "Erasing room" << room->id();
    const int row = it - m_rooms.begin();
    beginRemoveRows(QModelIndex(), row, row);
    m_rooms.erase(it);
    endRemoveRows();
}

int RoomListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rooms.count();
}

QVariant RoomListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_rooms.count()) {
        qDebug() << "UserListModel: something wrong here...";
        return QVariant();
    }
    NeoChatRoom *room = m_rooms.at(index.row());
    if (role == DisplayNameRole) {
        return room->displayName();
    }
    if (role == AvatarRole) {
        return room->avatarMediaId();
    }
    if (role == CanonicalAliasRole) {
        return room->canonicalAlias();
    }
    if (role == TopicRole) {
        return room->topic();
    }
    if (role == CategoryRole) {
        return NeoChatRoomType::typeForRoom(room);
    }
    if (role == NotificationCountRole) {
        return room->notificationCount();
    }
    if (role == HighlightCountRole) {
        return room->highlightCount();
    }
    if (role == LastActiveTimeRole) {
        return room->lastActiveTime();
    }
    if (role == JoinStateRole) {
        if (!room->successorId().isEmpty()) {
            return QStringLiteral("upgraded");
        }
        return QVariant::fromValue(room->joinState());
    }
    if (role == CurrentRoomRole) {
        return QVariant::fromValue(room);
    }
    if (role == SubtitleTextRole) {
        if (room->lastEvent() == nullptr || room->lastEventIsSpoiler()) {
            return QString();
        }
        EventHandler eventHandler(room, room->lastEvent());
        return eventHandler.subtitleText();
    }
    if (role == AvatarImageRole) {
        return room->avatar(128);
    }
    if (role == RoomIdRole) {
        return room->id();
    }
    if (role == IsSpaceRole) {
        return room->isSpace();
    }
    if (role == IsChildSpaceRole) {
        return SpaceHierarchyCache::instance().isChild(room->id());
    }
    if (role == ReplacementIdRole) {
        return room->successorId();
    }
    if (role == IsDirectChat) {
        return room->isDirectChat();
    }

    return QVariant();
}

void RoomListModel::refresh(NeoChatRoom *room, const QList<int> &roles)
{
    const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
    if (it == m_rooms.end()) {
        qCritical() << "Room" << room->id() << "not found in the room list";
        return;
    }
    const auto idx = index(it - m_rooms.begin());
    Q_EMIT dataChanged(idx, idx, roles);
}

QHash<int, QByteArray> RoomListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayNameRole] = "displayName";
    roles[AvatarRole] = "avatar";
    roles[CanonicalAliasRole] = "canonicalAlias";
    roles[TopicRole] = "topic";
    roles[CategoryRole] = "category";
    roles[NotificationCountRole] = "notificationCount";
    roles[HighlightCountRole] = "highlightCount";
    roles[LastActiveTimeRole] = "lastActiveTime";
    roles[JoinStateRole] = "joinState";
    roles[CurrentRoomRole] = "currentRoom";
    roles[SubtitleTextRole] = "subtitleText";
    roles[IsSpaceRole] = "isSpace";
    roles[RoomIdRole] = "roomId";
    roles[IsChildSpaceRole] = "isChildSpace";
    roles[IsDirectChat] = "isDirectChat";
    return roles;
}

NeoChatRoom *RoomListModel::roomByAliasOrId(const QString &aliasOrId)
{
    for (const auto &room : std::as_const(m_rooms)) {
        if (room->aliases().contains(aliasOrId) || room->id() == aliasOrId) {
            return room;
        }
    }
    return nullptr;
}

int RoomListModel::rowForRoom(NeoChatRoom *room) const
{
    return m_rooms.indexOf(room);
}

#include "moc_roomlistmodel.cpp"
