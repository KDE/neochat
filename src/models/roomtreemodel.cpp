// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "roomtreemodel.h"

#include <Quotient/connection.h>
#include <Quotient/room.h>

#include "eventhandler.h"
#include "neochatconnection.h"
#include "neochatroomtype.h"
#include "spacehierarchycache.h"

using namespace Quotient;

RoomTreeModel::RoomTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    initializeCategories();
}

void RoomTreeModel::initializeCategories()
{
    for (const auto &key : m_rooms.keys()) {
        for (const auto &room : m_rooms[key]) {
            room->disconnect(this);
        }
    }
    m_rooms.clear();
    for (int i = 0; i < 8; i++) {
        m_rooms[NeoChatRoomType::Types(i)] = {};
    }
}

void RoomTreeModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    if (m_connection) {
        disconnect(m_connection.get(), nullptr, this, nullptr);
    }
    m_connection = connection;
    beginResetModel();
    initializeCategories();
    endResetModel();
    connect(connection, &Connection::newRoom, this, &RoomTreeModel::newRoom);
    connect(connection, &Connection::leftRoom, this, &RoomTreeModel::leftRoom);
    connect(connection, &Connection::aboutToDeleteRoom, this, &RoomTreeModel::leftRoom);

    for (const auto &room : m_connection->allRooms()) {
        newRoom(dynamic_cast<NeoChatRoom *>(room));
    }
    Q_EMIT connectionChanged();
}

void RoomTreeModel::newRoom(Room *r)
{
    const auto room = dynamic_cast<NeoChatRoom *>(r);
    const auto type = NeoChatRoomType::typeForRoom(room);
    // Check if the room is already in the model.
    const auto checkRoomIndex = indexForRoom(room);
    if (checkRoomIndex.isValid()) {
        // If the room is in the wrong type category for whatever reason, move it.
        if (checkRoomIndex.parent().row() != type) {
            moveRoom(room);
        }
        return;
    }

    beginInsertRows(index(type, 0), m_rooms[type].size(), m_rooms[type].size());
    m_rooms[type].append(room);
    connectRoomSignals(room);
    endInsertRows();
}

void RoomTreeModel::leftRoom(Room *r)
{
    const auto room = dynamic_cast<NeoChatRoom *>(r);
    const auto type = NeoChatRoomType::typeForRoom(room);
    auto row = m_rooms[type].indexOf(room);
    if (row == -1) {
        return;
    }
    beginRemoveRows(index(type, 0), row, row);
    m_rooms[type][row]->disconnect(this);
    m_rooms[type].removeAt(row);
    endRemoveRows();
}

void RoomTreeModel::moveRoom(Quotient::Room *room)
{
    // We can't assume the type as it has changed so currently the return of
    // NeoChatRoomType::typeForRoom doesn't match it's current location. So find the room.
    NeoChatRoomType::Types oldType;
    int oldRow = -1;
    for (const auto &key : m_rooms.keys()) {
        if (m_rooms[key].contains(room)) {
            oldType = key;
            oldRow = m_rooms[key].indexOf(room);
        }
    }

    if (oldRow == -1) {
        return;
    }
    const auto newType = NeoChatRoomType::typeForRoom(dynamic_cast<NeoChatRoom *>(room));
    if (newType == oldType) {
        return;
    }

    const auto oldParent = index(oldType, 0, {});
    const auto newParent = index(newType, 0, {});
    // HACK: We're doing this as a remove then insert because  moving doesn't work
    // properly with DelegateChooser for whatever reason.
    beginRemoveRows(oldParent, oldRow, oldRow);
    m_rooms[oldType].removeAt(oldRow);
    endRemoveRows();
    beginInsertRows(newParent, m_rooms[newType].size(), m_rooms[newType].size());
    m_rooms[newType].append(dynamic_cast<NeoChatRoom *>(room));
    endInsertRows();
}

void RoomTreeModel::connectRoomSignals(NeoChatRoom *room)
{
    connect(room, &Room::displaynameChanged, this, [this, room] {
        refreshRoomRoles(room, {DisplayNameRole});
    });
    connect(room, &Room::unreadStatsChanged, this, [this, room] {
        refreshRoomRoles(room, {NotificationCountRole, HighlightCountRole});
    });
    connect(room, &Room::avatarChanged, this, [this, room] {
        refreshRoomRoles(room, {AvatarRole});
    });
    connect(room, &Room::tagsChanged, this, [this, room] {
        moveRoom(room);
    });
    connect(room, &Room::joinStateChanged, this, [this, room] {
        refreshRoomRoles(room);
    });
    connect(room, &Room::addedMessages, this, [this, room] {
        refreshRoomRoles(room, {SubtitleTextRole, LastActiveTimeRole});
    });
    connect(room, &Room::pendingEventMerged, this, [this, room] {
        refreshRoomRoles(room, {SubtitleTextRole});
    });
}

void RoomTreeModel::refreshRoomRoles(NeoChatRoom *room, const QList<int> &roles)
{
    const auto roomType = NeoChatRoomType::typeForRoom(room);
    const auto it = std::find(m_rooms[roomType].begin(), m_rooms[roomType].end(), room);
    if (it == m_rooms[roomType].end()) {
        qCritical() << "Room" << room->id() << "not found in the room list";
        return;
    }
    const auto parentIndex = index(roomType, 0, {});
    const auto idx = index(it - m_rooms[roomType].begin(), 0, parentIndex);
    Q_EMIT dataChanged(idx, idx, roles);
}

NeoChatConnection *RoomTreeModel::connection() const
{
    return m_connection;
}

int RoomTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

int RoomTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_rooms.keys().size();
    }
    if (!parent.parent().isValid()) {
        return m_rooms.values()[parent.row()].size();
    }
    return 0;
}

QModelIndex RoomTreeModel::parent(const QModelIndex &index) const
{
    if (!index.internalPointer()) {
        return {};
    }
    return this->index(NeoChatRoomType::typeForRoom(static_cast<NeoChatRoom *>(index.internalPointer())), 0, QModelIndex());
}

QModelIndex RoomTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column, nullptr);
    }
    if (row >= rowCount(parent)) {
        return {};
    }
    return createIndex(row, column, m_rooms[NeoChatRoomType::Types(parent.row())][row]);
}

QHash<int, QByteArray> RoomTreeModel::roleNames() const
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
    roles[DelegateTypeRole] = "delegateType";
    roles[IconRole] = "icon";
    roles[AttentionRole] = "attention";
    roles[FavouriteRole] = "favourite";
    return roles;
}

// TODO room type changes
QVariant RoomTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (role == DisplayNameRole) {
            return NeoChatRoomType::typeName(index.row());
        }
        if (role == DelegateTypeRole) {
            if (index.row() == NeoChatRoomType::Search) {
                return QStringLiteral("search");
            }
            if (index.row() == NeoChatRoomType::AddDirect) {
                return QStringLiteral("addDirect");
            }
            return QStringLiteral("section");
        }
        if (role == IconRole) {
            return NeoChatRoomType::typeIconName(index.row());
        }
        if (role == CategoryRole) {
            return index.row();
        }
        return {};
    }
    const auto room = m_rooms.values()[index.parent().row()][index.row()].get();
    Q_ASSERT(room);

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
        return int(room->notificationCount());
    }
    if (role == HighlightCountRole) {
        return int(room->highlightCount());
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
    if (role == DelegateTypeRole) {
        return QStringLiteral("normal");
    }
    if (role == AttentionRole) {
        return room->notificationCount() + room->highlightCount() > 0;
    }
    if (role == FavouriteRole) {
        return room->isFavourite();
    }

    return {};
}

QModelIndex RoomTreeModel::indexForRoom(NeoChatRoom *room) const
{
    if (room == nullptr) {
        return {};
    }

    // Try and find by checking type.
    const auto type = NeoChatRoomType::typeForRoom(room);
    auto row = m_rooms[type].indexOf(room);
    if (row >= 0) {
        return index(row, 0, index(type, 0));
    }
    // Double check that the room isn't in the wrong category.
    for (const auto &key : m_rooms.keys()) {
        if (m_rooms[key].contains(room)) {
            return index(m_rooms[key].indexOf(room), 0, index(key, 0));
        }
    }
    return {};
}

#include "moc_roomtreemodel.cpp"
