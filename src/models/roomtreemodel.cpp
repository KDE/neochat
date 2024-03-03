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

TreeItem::TreeItem(TreeData treeData, TreeItem *parent)
    : m_treeData(treeData)
    , m_parentItem(parent)
{
}

void TreeItem::appendChild(std::unique_ptr<TreeItem> &&child)
{
    m_childItems.push_back(std::move(child));
}

bool TreeItem::insertChildren(int position, int count, TreeData treeData)
{
    if (position < 0 || position > qsizetype(m_childItems.size()))
        return false;

    for (int row = 0; row < count; ++row) {
        m_childItems.insert(m_childItems.cbegin() + position, std::make_unique<TreeItem>(treeData, this));
    }

    return true;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > qsizetype(m_childItems.size())) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        m_childItems.erase(m_childItems.cbegin() + position);
    }

    return true;
}

TreeItem *TreeItem::child(int row)
{
    return row >= 0 && row < childCount() ? m_childItems.at(row).get() : nullptr;
}

int TreeItem::childCount() const
{
    return int(m_childItems.size());
}

int TreeItem::row() const
{
    if (m_parentItem == nullptr) {
        return 0;
    }
    const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(), [this](const std::unique_ptr<TreeItem> &treeItem) {
        return treeItem.get() == this;
    });

    if (it != m_parentItem->m_childItems.cend())
        return std::distance(m_parentItem->m_childItems.cbegin(), it);
    Q_ASSERT(false); // should not happen
    return -1;
}

QVariant TreeItem::data(int role) const
{
    if (!m_parentItem) {
        return {};
    }

    if (std::holds_alternative<NeoChatRoomType::Types>(m_treeData)) {
        const auto row = this->row();
        switch (role) {
        case RoomTreeModel::DisplayNameRole:
            return NeoChatRoomType::typeName(row);
        case RoomTreeModel::DelegateTypeRole:
            if (row == NeoChatRoomType::Search) {
                return QStringLiteral("search");
            }
            if (row == NeoChatRoomType::AddDirect) {
                return QStringLiteral("addDirect");
            }
            return QStringLiteral("section");
        case RoomTreeModel::IconRole:
            return NeoChatRoomType::typeIconName(row);
        case RoomTreeModel::CategoryRole:
            return row;
        default:
            return {};
        }
    }

    const auto room = std::get<NeoChatRoom *>(m_treeData);

    switch (role) {
    case RoomTreeModel::DisplayNameRole:
        return room->displayName();
    case RoomTreeModel::AvatarRole:
        return room->avatarMediaId();
    case RoomTreeModel::CanonicalAliasRole:
        return room->canonicalAlias();
    case RoomTreeModel::TopicRole:
        return room->topic();
    case RoomTreeModel::CategoryRole:
        return NeoChatRoomType::typeForRoom(room);
    case RoomTreeModel::ContextNotificationCountRole:
        return room->contextAwareNotificationCount();
    case RoomTreeModel::HasHighlightNotificationsRole:
        return room->highlightCount() > 0 && room->contextAwareNotificationCount() > 0;
    case RoomTreeModel::LastActiveTimeRole:
        return room->lastActiveTime();
    case RoomTreeModel::JoinStateRole:
        if (!room->successorId().isEmpty()) {
            return QStringLiteral("upgraded");
        }
        return QVariant::fromValue(room->joinState());
    case RoomTreeModel::CurrentRoomRole:
        return QVariant::fromValue(room);
    case RoomTreeModel::SubtitleTextRole: {
        if (room->lastEvent() == nullptr || room->lastEventIsSpoiler()) {
            return QString();
        }
        EventHandler eventHandler(room, room->lastEvent());
        return eventHandler.subtitleText();
    }
    case RoomTreeModel::AvatarImageRole:
        return room->avatar(128);
    case RoomTreeModel::RoomIdRole:
        return room->id();
    case RoomTreeModel::IsSpaceRole:
        return room->isSpace();
    case RoomTreeModel::IsChildSpaceRole:
        return SpaceHierarchyCache::instance().isChild(room->id());
    case RoomTreeModel::ReplacementIdRole:
        return room->successorId();
    case RoomTreeModel::IsDirectChat:
        return room->isDirectChat();
    case RoomTreeModel::DelegateTypeRole:
        return QStringLiteral("normal");
    }

    return {};
}

TreeItem *TreeItem::parentItem() const
{
    return m_parentItem;
}

std::optional<int> TreeItem::position(Quotient::Room *room) const
{
    Q_ASSERT_X(std::holds_alternative<NeoChatRoomType::Types>(m_treeData), __FUNCTION__, "containsRoom only works in category items");

    int i = 0;
    for (const auto &child : m_childItems) {
        if (std::get<NeoChatRoom *>(child->treeData()) == room) {
            return i;
        }
        i++;
    }

    return std::nullopt;
}

TreeItem::TreeData TreeItem::treeData() const
{
    return m_treeData;
}

RoomTreeModel::RoomTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    initializeCategories();
}

void RoomTreeModel::initializeCategories()
{
    m_rootItem.reset(new TreeItem(nullptr));
    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        m_rootItem->appendChild(std::make_unique<TreeItem>(NeoChatRoomType::Types(i), m_rootItem.get()));
    }
}

TreeItem *RoomTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<TreeItem *>(index.internalPointer());
    }
    return m_rootItem.get();
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

    auto categoryItem = m_rootItem->child(type);
    beginInsertRows(index(type, 0), categoryItem->childCount(), categoryItem->childCount());
    categoryItem->appendChild(std::make_unique<TreeItem>(room));
    qWarning() << "append" << index(type, 0) << categoryItem->childCount();
    connectRoomSignals(room);
    endInsertRows();
}

void RoomTreeModel::leftRoom(Room *r)
{
    const auto room = dynamic_cast<NeoChatRoom *>(r);

    auto idx = indexForRoom(room);
    if (!idx.isValid()) {
        return;
    }

    auto parentItem = getItem(idx.parent());
    Q_ASSERT(parentItem);

    beginRemoveRows(idx.parent(), idx.row(), idx.row());
    const bool success = parentItem->removeChildren(idx.row(), 1);
    room->disconnect(this);
    endRemoveRows();

    if (success) {
        qWarning() << "Unable to remove room";
    }
}

void RoomTreeModel::moveRoom(Quotient::Room *room)
{
    // We can't assume the type as it has changed so currently the return of
    // NeoChatRoomType::typeForRoom doesn't match it's current location. So find the room.
    NeoChatRoomType::Types oldType;
    int oldRow = -1;
    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        auto categoryItem = m_rootItem->child(i);
        auto position = categoryItem->position(room);
        if (position) {
            oldType = static_cast<NeoChatRoomType::Types>(i);
            oldRow = *position;
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

    beginRemoveRows(oldParent, oldRow, oldRow);
    const bool success = oldParentItem->removeChildren(oldRow, 1);
    Q_ASSERT(success);
    endRemoveRows();

    beginInsertRows(newParent, newParentItem->childCount(), newParentItem->childCount());
    newParentItem->appendChild(std::make_unique<TreeItem>(neochatRoom));
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
    const auto idx = indexForRoom(room);
    if (!idx.isValid()) {
        qCritical() << "Room" << room->id() << "not found in the room list";
        return;
    }

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
    const TreeItem *parentItem = getItem(parent);
    return parentItem ? parentItem->childCount() : 0;
}

QModelIndex RoomTreeModel::parent(const QModelIndex &index) const
{
    qWarning() << "getting parent from" << index;
    if (!index.isValid()) {
        return {};
    }

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem ? childItem->parentItem() : nullptr;

    qWarning() << parentItem << m_rootItem.get();

    return (parentItem != m_rootItem.get() && parentItem != nullptr) ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

QModelIndex RoomTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return {};
    }

    TreeItem *parentItem = getItem(parent);
    Q_ASSERT(parentItem);

    if (auto *childItem = parentItem->child(row)) {
        return createIndex(row, column, childItem);
    }
    return {};
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
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    return getItem(index)->data(role);
}

QModelIndex RoomTreeModel::indexForRoom(NeoChatRoom *room) const
{
    if (room == nullptr) {
        return {};
    }

    const auto roomType = NeoChatRoomType::typeForRoom(room);
    const auto roomTypeItem = m_rootItem->child(roomType);

    for (int i = 0, count = roomTypeItem->childCount(); i < count; i++) {
        auto roomItem = roomTypeItem->child(i);
        if (std::get<NeoChatRoom *>(roomItem->treeData()) == room) {
            const auto parentIndex = index(roomType, 0, {});
            const auto idx = index(i, 0, parentIndex);
            return idx;
        }
    }

    return {};
}

#include "moc_roomtreemodel.cpp"
