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
    , m_rootItem(new RoomTreeItem(nullptr))
{
}

RoomTreeItem *RoomTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        RoomTreeItem *item = static_cast<RoomTreeItem *>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return m_rootItem.get();
}

void RoomTreeModel::resetModel()
{
    if (m_connection == nullptr) {
        beginResetModel();
        m_rootItem.reset();
        endResetModel();
        return;
    }

    beginResetModel();
    m_rootItem.reset(new RoomTreeItem(nullptr));

    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        m_rootItem->insertChild(std::make_unique<RoomTreeItem>(NeoChatRoomType::Types(i), m_rootItem.get()));
    }

    for (const auto &r : m_connection->allRooms()) {
        const auto room = dynamic_cast<NeoChatRoom *>(r);
        const auto type = NeoChatRoomType::typeForRoom(room);
        const auto categoryItem = m_rootItem->child(type);
        if (categoryItem->insertChild(std::make_unique<RoomTreeItem>(room, categoryItem))) {
            connectRoomSignals(room);
        }
    }

    endResetModel();
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

    resetModel();

    connect(connection, &Connection::newRoom, this, &RoomTreeModel::newRoom);
    connect(connection, &Connection::leftRoom, this, &RoomTreeModel::leftRoom);
    connect(connection, &Connection::aboutToDeleteRoom, this, &RoomTreeModel::leftRoom);

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

    const auto parentItem = m_rootItem->child(type);
    beginInsertRows(index(parentItem->row(), 0), parentItem->childCount(), parentItem->childCount());
    parentItem->insertChild(std::make_unique<RoomTreeItem>(room, parentItem));
    connectRoomSignals(room);
    endInsertRows();
    qWarning() << "adding room" << type << "new count" << parentItem->childCount();
}

void RoomTreeModel::leftRoom(Room *r)
{
    const auto room = dynamic_cast<NeoChatRoom *>(r);
    auto index = indexForRoom(room);
    if (!index.isValid()) {
        return;
    }

    const auto parentItem = getItem(index.parent());
    Q_ASSERT(parentItem);

    beginRemoveRows(index.parent(), index.row(), index.row());
    parentItem->removeChild(index.row());
    room->disconnect(this);
    endRemoveRows();
}

void RoomTreeModel::moveRoom(Quotient::Room *room)
{
    // We can't assume the type as it has changed so currently the return of
    // NeoChatRoomType::typeForRoom doesn't match it's current location. So find the room.
    NeoChatRoomType::Types oldType;
    int oldRow = -1;
    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        const auto categoryItem = m_rootItem->child(i);
        const auto row = categoryItem->rowForRoom(room);
        if (row) {
            oldType = static_cast<NeoChatRoomType::Types>(i);
            oldRow = *row;
        }
    }

    if (oldRow == -1) {
        return;
    }
    auto neochatRoom = dynamic_cast<NeoChatRoom *>(room);
    const auto newType = NeoChatRoomType::typeForRoom(neochatRoom);
    if (newType == oldType) {
        return;
    }

    const auto oldParent = index(oldType, 0, {});
    auto oldParentItem = getItem(oldParent);
    Q_ASSERT(oldParentItem);

    const auto newParent = index(newType, 0, {});
    auto newParentItem = getItem(newParent);
    Q_ASSERT(newParentItem);

    // HACK: We're doing this as a remove then insert because  moving doesn't work
    // properly with DelegateChooser for whatever reason.
    Q_ASSERT(checkIndex(index(oldRow, 0, oldParent), QAbstractItemModel::CheckIndexOption::IndexIsValid));
    beginRemoveRows(oldParent, oldRow, oldRow);
    const bool success = oldParentItem->removeChild(oldRow);
    Q_ASSERT(success);
    endRemoveRows();
    beginInsertRows(newParent, newParentItem->childCount(), newParentItem->childCount());
    newParentItem->insertChild(std::make_unique<RoomTreeItem>(neochatRoom, newParentItem));
    endInsertRows();
}

void RoomTreeModel::connectRoomSignals(NeoChatRoom *room)
{
    connect(room, &Room::displaynameChanged, this, [this, room] {
        refreshRoomRoles(room, {DisplayNameRole});
    });
    connect(room, &Room::unreadStatsChanged, this, [this, room] {
        refreshRoomRoles(room, {ContextNotificationCountRole, HasHighlightNotificationsRole});
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
    connect(room, &NeoChatRoom::pushNotificationStateChanged, this, [this, room] {
        refreshRoomRoles(room, {ContextNotificationCountRole, HasHighlightNotificationsRole});
    });
}

void RoomTreeModel::refreshRoomRoles(NeoChatRoom *room, const QList<int> &roles)
{
    const auto index = indexForRoom(room);
    if (!index.isValid()) {
        qCritical() << "Room" << room->id() << "not found in the room list";
        return;
    }
    Q_EMIT dataChanged(index, index, roles);
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
    RoomTreeItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem.get();
    } else {
        parentItem = static_cast<RoomTreeItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

QModelIndex RoomTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    RoomTreeItem *childItem = static_cast<RoomTreeItem *>(index.internalPointer());
    if (!childItem) {
        return QModelIndex();
    }
    RoomTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem.get()) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QModelIndex RoomTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    RoomTreeItem *parentItem = getItem(parent);
    if (!parentItem) {
        return QModelIndex();
    }

    RoomTreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QHash<int, QByteArray> RoomTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayNameRole] = "displayName";
    roles[AvatarRole] = "avatar";
    roles[CanonicalAliasRole] = "canonicalAlias";
    roles[TopicRole] = "topic";
    roles[CategoryRole] = "category";
    roles[ContextNotificationCountRole] = "contextNotificationCount";
    roles[HasHighlightNotificationsRole] = "hasHighlightNotifications";
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
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    RoomTreeItem *child = getItem(index);
    if (std::holds_alternative<NeoChatRoomType::Types>(child->data())) {
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

    const auto room = std::get<NeoChatRoom *>(child->data());
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
    if (role == ContextNotificationCountRole) {
        return int(room->contextAwareNotificationCount());
    }
    if (role == HasHighlightNotificationsRole) {
        return room->highlightCount() > 0 && room->contextAwareNotificationCount() > 0;
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
    const auto parentItem = m_rootItem->child(type);
    const auto row = parentItem->rowForRoom(room);
    if (row) {
        return index(*row, 0, index(type, 0));
    }
    // Double check that the room isn't in the wrong category.
    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        const auto parentItem = m_rootItem->child(i);
        const auto row = parentItem->rowForRoom(room);
        if (row) {
            return index(*row, 0, index(i, 0));
        }
    }

    return {};
}

#include "moc_roomtreemodel.cpp"
