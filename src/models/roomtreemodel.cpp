
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "roomtreemodel.h"

#include "neochatconnection.h"
// #include "eventhandler.h"
#include "neochatroomtype.h"
#include "rust/cxx.h"
#include <Integral/lib.rs.h>
// #include "spacehierarchycache.h"
#include <Integral/RoomStream>
#include <Integral/Utils>

using namespace Integral;

class RoomTreeModel::Private
{
public:
    QPointer<Integral::Connection> connection;
    std::unique_ptr<RoomStream> roomStream = nullptr;
    std::unique_ptr<RoomTreeItem> rootItem;
    // Since the rooms are streamed as vector diffs we need to keep track of them
    // for things like the index value of insert to make sense.
    QList<QPersistentModelIndex> roomIndexes;

    void roomsUpdate();
    void resetTree();

    RoomTreeModel *q = nullptr;
};

RoomTreeModel::RoomTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(std::make_unique<Private>())
{
    d->q = this;
}

RoomTreeModel::~RoomTreeModel() = default;

RoomTreeItem *RoomTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        RoomTreeItem *item = static_cast<RoomTreeItem *>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return d->rootItem.get();
}

void RoomTreeModel::resetModel()
{
    if (d->connection == nullptr) {
        beginResetModel();
        d->rootItem.reset();
        d->roomStream.reset();
        endResetModel();
        return;
    }

    beginResetModel();
    d->resetTree();

    d->roomStream = d->connection->roomStream();
    connect(d->roomStream.get(), &RoomStream::roomsUpdate, this, [this]() {
        d->roomsUpdate();
    });

    d->roomStream->startStream();

    endResetModel();
}

void RoomTreeModel::Private::resetTree()
{
    rootItem.reset(new RoomTreeItem(nullptr));
    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        rootItem->insertChild(std::make_unique<RoomTreeItem>(NeoChatRoomType::Type(i), rootItem.get()));
    }
}

void RoomTreeModel::setConnection(Connection *connection)
{
    if (d->connection == connection) {
        return;
    }
    if (d->connection) {
        d->connection->disconnect(this);
    }
    d->connection = connection;

    resetModel();
    Q_EMIT connectionChanged();
}

void RoomTreeModel::Private::roomsUpdate()
{
    const auto diff = roomStream->next();

    switch (diff->op()) {
    case 0: { // Append
        for (const auto &it : diff->items_vec()) {
            const auto type = NeoChatRoomType::typeForRoom(it.box_me());
            const auto parentItem = rootItem->child(type);
            q->beginInsertRows(q->index(parentItem->row(), 0), parentItem->childCount(), parentItem->childCount());
            if (parentItem->insertChild(std::make_unique<RoomTreeItem>(new RoomWrapper{it.box_me()}, parentItem))) {
                // connectRoomSignals(room);
            }
            q->endInsertRows();
            roomIndexes.append(q->indexForRoom(it.box_me()));
        }
        break;
    }
    case 1: { // Clear
        q->beginResetModel();
        resetTree();
        roomIndexes.clear();
        q->endResetModel();
        break;
    }
    case 2: { // Push Front
        const auto type = NeoChatRoomType::typeForRoom(diff->item());
        const auto parentItem = rootItem->child(type);
        q->beginInsertRows(q->index(parentItem->row(), 0), 0, 0);
        if (parentItem->insertChild(std::make_unique<RoomTreeItem>(new RoomWrapper{diff->item()}, parentItem))) {
            // connectRoomSignals(room);
        }
        q->endInsertRows();
        roomIndexes.prepend(q->indexForRoom(diff->item()));
        break;
    }
    case 3: { // Push Back
        const auto type = NeoChatRoomType::typeForRoom(diff->item());
        const auto parentItem = rootItem->child(type);
        q->beginInsertRows(q->index(parentItem->row(), 0), parentItem->childCount(), parentItem->childCount());
        if (parentItem->insertChild(std::make_unique<RoomTreeItem>(new RoomWrapper{diff->item()}, parentItem))) {
            // connectRoomSignals(room);
        }
        q->endInsertRows();
        roomIndexes.append(q->indexForRoom(diff->item()));
        break;
    }
    case 4: { // Pop Front
        const auto index = roomIndexes.front();
        q->beginRemoveRows(index.parent(), index.row(), index.row());
        const auto parentItem = q->getItem(index.parent());
        parentItem->removeChild(index.row());
        roomIndexes.removeFirst();
        q->endRemoveRows();
        break;
    }
    case 5: { // Pop Back
        const auto index = roomIndexes.back();
        q->beginRemoveRows(index.parent(), index.row(), index.row());
        const auto parentItem = q->getItem(index.parent());
        parentItem->removeChild(index.row());
        roomIndexes.removeLast();
        q->endRemoveRows();
        break;
    }
    case 6: { // Insert
        const auto type = NeoChatRoomType::typeForRoom(diff->item());
        const auto parentItem = rootItem->child(type);
        q->beginInsertRows(q->index(parentItem->row(), 0), parentItem->childCount(), parentItem->childCount());
        if (parentItem->insertChild(std::make_unique<RoomTreeItem>(new RoomWrapper{diff->item()}, parentItem))) {
            // connectRoomSignals(room);
        }
        q->endInsertRows();
        roomIndexes.insert(diff->index(), q->indexForRoom(diff->item()));
        break;
    }
    case 7: { // Set
        const auto index = roomIndexes.at(diff->index());
        q->beginRemoveRows(index.parent(), index.row(), index.row());
        q->getItem(index.parent())->removeChild(index.row());
        q->endRemoveRows();

        const auto type = NeoChatRoomType::typeForRoom(diff->item());
        const auto parentItem = rootItem->child(type);
        q->beginInsertRows(q->index(parentItem->row(), 0), parentItem->childCount(), parentItem->childCount());
        if (parentItem->insertChild(std::make_unique<RoomTreeItem>(new RoomWrapper{diff->item()}, parentItem))) {
            // connectRoomSignals(room);
        }
        q->endInsertRows();
        roomIndexes[diff->index()] = q->indexForRoom(diff->item());
        break;
    }
    case 8: { // Remove
        const auto index = roomIndexes.at(diff->index());
        q->beginRemoveRows(index.parent(), index.row(), index.row());
        q->getItem(index.parent())->removeChild(index.row());
        q->endRemoveRows();
        roomIndexes.removeAt(diff->index());
        break;
    }
    case 9: { // Truncate
        for (int i = q->rowCount({}) - 1; i >= int(diff->index()); i--) {
            const auto index = roomIndexes.at(i);
            q->beginRemoveRows(index.parent(), index.row(), index.row());
            q->getItem(index.parent())->removeChild(index.row());
            q->endRemoveRows();
            roomIndexes.removeAt(i);
        }
        break;
    }
    case 10: { // Reset
        q->beginResetModel();
        resetTree();
        roomIndexes.clear();
        q->endResetModel();

        for (const auto &it : diff->items_vec()) {
            const auto type = NeoChatRoomType::typeForRoom(it.box_me());
            const auto parentItem = rootItem->child(type);
            q->beginInsertRows(q->index(parentItem->row(), 0), parentItem->childCount(), parentItem->childCount());
            if (parentItem->insertChild(std::make_unique<RoomTreeItem>(new RoomWrapper{it.box_me()}, parentItem))) {
                // connectRoomSignals(room);
            }
            q->endInsertRows();
            roomIndexes.append(q->indexForRoom(it.box_me()));
        }
        break;
    }
    }
}

// void RoomTreeModel::connectRoomSignals(NeoChatRoom *room)
// {
//     connect(room, &Room::displaynameChanged, this, [this, room] {
//         refreshRoomRoles(room, {DisplayNameRole});
//     });
//     connect(room, &Room::unreadStatsChanged, this, [this, room] {
//         refreshRoomRoles(room, {ContextNotificationCountRole, HasHighlightNotificationsRole});
//     });
//     connect(room, &Room::avatarChanged, this, [this, room] {
//         refreshRoomRoles(room, {AvatarRole});
//     });
//     connect(room, &Room::tagsChanged, this, [this, room] {
//         moveRoom(room);
//     });
//     connect(room, &Room::joinStateChanged, this, [this, room] {
//         refreshRoomRoles(room);
//     });
//     connect(room, &Room::addedMessages, this, [this, room] {
//         refreshRoomRoles(room, {SubtitleTextRole});
//     });
//     connect(room, &Room::pendingEventMerged, this, [this, room] {
//         refreshRoomRoles(room, {SubtitleTextRole});
//     });
//     connect(room, &NeoChatRoom::pushNotificationStateChanged, this, [this, room] {
//         refreshRoomRoles(room, {ContextNotificationCountRole, HasHighlightNotificationsRole});
//     });
// }

// void RoomTreeModel::refreshRoomRoles(NeoChatRoom *room, const QList<int> &roles)
// {
//     const auto index = indexForRoom(room);
//     if (!index.isValid()) {
//         qCritical() << "Room" << room->id() << "not found in the room list";
//         return;
//     }
//     Q_EMIT dataChanged(index, index, roles);
// }

Connection *RoomTreeModel::connection() const
{
    return d->connection;
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
        parentItem = d->rootItem.get();
    } else {
        parentItem = static_cast<RoomTreeItem *>(parent.internalPointer());
    }

    if (!parentItem) {
        return 0;
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

    if (parentItem == d->rootItem.get()) {
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
    roles[JoinStateRole] = "joinState";
    roles[CurrentRoomRole] = "currentRoom";
    roles[SubtitleTextRole] = "subtitleText";
    roles[IsSpaceRole] = "isSpace";
    roles[RoomIdRole] = "roomId";
    roles[IsChildSpaceRole] = "isChildSpace";
    roles[IsDirectChat] = "isDirectChat";
    roles[DelegateTypeRole] = "delegateType";
    roles[IconRole] = "icon";
    roles[RoomTypeRole] = "roomType";
    return roles;
}

// TODO room type changes
QVariant RoomTreeModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    RoomTreeItem *child = getItem(index);
    if (std::holds_alternative<NeoChatRoomType::Type>(child->data())) {
        if (role == DisplayNameRole) {
            return NeoChatRoomType::typeName(index.row());
        }
        if (role == DelegateTypeRole) {
            if (index.row() == NeoChatRoomType::AddDirect) {
                return u"addDirect"_s;
            }
            return u"section"_s;
        }
        if (role == IconRole) {
            return NeoChatRoomType::typeIconName(index.row());
        }
        if (role == CategoryRole) {
            return index.row();
        }
        return {};
    }

    const auto room = std::get<RoomWrapper *>(child->data());
    Q_ASSERT(room);

    if (role == DisplayNameRole) {
        return stringFromRust((*room->item)->display_name()).toHtmlEscaped();
    }
    if (role == AvatarRole) {
        return u"%1?user_id=%2"_s.arg(stringFromRust((*room->item)->avatar_url()), d->connection->matrixId());
    }
    if (role == CanonicalAliasRole) {
        return stringFromRust((*room->item)->canonical_alias()).toHtmlEscaped();
    }
    if (role == TopicRole) {
        return stringFromRust((*room->item)->topic()).toHtmlEscaped();
    }
    if (role == CategoryRole) {
        return NeoChatRoomType::typeForRoom((*room->item)->box_me());
    }
    if (role == ContextNotificationCountRole) {
        return int((*room->item)->num_unread_messages());
    }
    if (role == HasHighlightNotificationsRole) {
        return (*room->item)->num_unread_mentions() > 0 && (*room->item)->num_unread_messages() > 0;
    }
    if (role == JoinStateRole) {
        if (!(*room->item)->tombstone()->replacement_room().empty()) {
            return u"upgraded"_s;
        }
        return QVariant::fromValue((*room->item)->state());
    }
    if (role == CurrentRoomRole) {
        return {};
        // return QVariant::fromValue(room);
    }
    if (role == SubtitleTextRole) {
        return {};
        // if (room->lastEvent() == nullptr || room->lastEventIsSpoiler()) {
        //     return QString();
        // }
        // return EventHandler::subtitleText(room, room->lastEvent());
    }
    if (role == AvatarImageRole) {
        return {};
        // return room->avatar(128);
    }
    if (role == RoomIdRole) {
        return stringFromRust((*room->item)->id()).toHtmlEscaped();
    }
    if (role == IsSpaceRole) {
        return (*room->item)->is_space();
    }
    if (role == IsChildSpaceRole) {
        return false;
        // return SpaceHierarchyCache::instance().isChild(room->id());
    }
    if (role == ReplacementIdRole) {
        return stringFromRust((*room->item)->tombstone()->replacement_room()).toHtmlEscaped();
    }
    if (role == IsDirectChat) {
        return false;
        // return room->isDirectChat();
    }
    if (role == DelegateTypeRole) {
        return u"normal"_s;
    }
    if (role == RoomTypeRole) {
        return stringFromRust((*room->item)->room_type()).toHtmlEscaped();
    }

    return {};
}

QModelIndex RoomTreeModel::indexForRoom(rust::Box<sdk::RoomListRoom> room) const
{
    // Try and find by checking type.
    const auto type = NeoChatRoomType::typeForRoom(room->box_me());
    const auto parentItem = d->rootItem->child(type);
    const auto row = parentItem->rowForRoom(room->box_me());
    if (row) {
        return index(*row, 0, index(type, 0));
    }
    // Double check that the room isn't in the wrong category.
    for (int i = 0; i < NeoChatRoomType::TypesCount; i++) {
        const auto parentItem = d->rootItem->child(i);
        const auto row = parentItem->rowForRoom(room->box_me());
        if (row) {
            return index(*row, 0, index(i, 0));
        }
    }

    return {};
}

std::optional<rust::Box<sdk::RoomListRoom>> RoomTreeModel::roomForIndex(QModelIndex index) const
{
    RoomTreeItem *child = getItem(index);
    if (std::holds_alternative<NeoChatRoomType::Type>(child->data())) {
        return std::nullopt;
    }
    return (*std::get<RoomWrapper *>(child->data())->item)->box_me();
}

#include "moc_roomtreemodel.cpp"
