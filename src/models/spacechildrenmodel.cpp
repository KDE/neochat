// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "spacechildrenmodel.h"

#include <Quotient/connection.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/room.h>

#include "neochatconnection.h"

SpaceChildrenModel::SpaceChildrenModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new SpaceTreeItem(nullptr);
}

SpaceChildrenModel::~SpaceChildrenModel()
{
    delete m_rootItem;
}

NeoChatRoom *SpaceChildrenModel::space() const
{
    return m_space;
}

void SpaceChildrenModel::setSpace(NeoChatRoom *space)
{
    if (space == m_space) {
        return;
    }
    // disconnect the new room signal from the old connection in case it is different.
    if (m_space != nullptr) {
        disconnect(m_space->connection(), &Quotient::Connection::loadedRoomState, this, nullptr);
    }

    m_space = space;
    Q_EMIT spaceChanged();

    for (auto job : m_currentJobs) {
        if (job) {
            job->abandon();
        }
    }
    m_currentJobs.clear();

    auto connection = m_space->connection();
    connect(connection, &Quotient::Connection::loadedRoomState, this, [this](Quotient::Room *room) {
        if (m_pendingChildren.contains(room->name())) {
            m_pendingChildren.removeAll(room->name());
            refreshModel();
        }
    });
    connect(m_space, &Quotient::Room::changed, this, [this]() {
        refreshModel();
    });

    refreshModel();
}

bool SpaceChildrenModel::loading() const
{
    return m_loading;
}

void SpaceChildrenModel::refreshModel()
{
    beginResetModel();
    m_replacedRooms.clear();
    delete m_rootItem;
    m_loading = true;
    Q_EMIT loadingChanged();
    m_rootItem =
        new SpaceTreeItem(dynamic_cast<NeoChatConnection *>(m_space->connection()), nullptr, m_space->id(), m_space->displayName(), m_space->canonicalAlias());
    endResetModel();
    auto job = m_space->connection()->callApi<Quotient::GetSpaceHierarchyJob>(m_space->id(), Quotient::none, Quotient::none, 1);
    m_currentJobs.append(job);
    connect(job, &Quotient::BaseJob::success, this, [this, job]() {
        insertChildren(job->rooms());
    });
}

void SpaceChildrenModel::insertChildren(std::vector<Quotient::GetSpaceHierarchyJob::ChildRoomsChunk> children, const QModelIndex &parent)
{
    SpaceTreeItem *parentItem = getItem(parent);

    if (children[0].roomId == m_space->id() || children[0].roomId == parentItem->id()) {
        parentItem->setChildStates(std::move(children[0].childrenState));
        children.erase(children.begin());
    }

    // If this is the first set of children added to the root item then we need to
    // set it so that we are no longer loading.
    if (rowCount(QModelIndex()) == 0 && !children.empty()) {
        m_loading = false;
        Q_EMIT loadingChanged();
    }

    beginInsertRows(parent, parentItem->childCount(), parentItem->childCount() + children.size() - 1);
    for (unsigned long i = 0; i < children.size(); ++i) {
        if (children[i].roomId == m_space->id() || children[i].roomId == parentItem->id()) {
            continue;
        } else {
            int insertRow = parentItem->childCount();
            if (const auto room = m_space->connection()->room(children[i].roomId)) {
                const auto predecessorId = room->predecessorId();
                if (!predecessorId.isEmpty()) {
                    m_replacedRooms += predecessorId;
                }
                const auto successorId = room->successorId();
                if (!successorId.isEmpty()) {
                    m_replacedRooms += successorId;
                }
            }
            if (children[i].childrenState.size() > 0) {
                auto job = m_space->connection()->callApi<Quotient::GetSpaceHierarchyJob>(children[i].roomId, Quotient::none, Quotient::none, 1);
                m_currentJobs.append(job);
                connect(job, &Quotient::BaseJob::success, this, [this, parent, insertRow, job]() {
                    insertChildren(job->rooms(), index(insertRow, 0, parent));
                });
            }
            parentItem->insertChild(insertRow,
                                    new SpaceTreeItem(dynamic_cast<NeoChatConnection *>(m_space->connection()),
                                                      parentItem,
                                                      children[i].roomId,
                                                      children[i].name,
                                                      children[i].canonicalAlias,
                                                      children[i].topic,
                                                      children[i].numJoinedMembers,
                                                      children[i].avatarUrl,
                                                      children[i].guestCanJoin,
                                                      children[i].worldReadable,
                                                      children[i].roomType == QLatin1String("m.space"),
                                                      std::move(children[i].childrenState)));
        }
    }
    endInsertRows();
}

SpaceTreeItem *SpaceChildrenModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        SpaceTreeItem *item = static_cast<SpaceTreeItem *>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return m_rootItem;
}

QVariant SpaceChildrenModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    SpaceTreeItem *child = getItem(index);
    if (role == DisplayNameRole) {
        auto displayName = child->name();
        if (!displayName.isEmpty()) {
            return displayName;
        }

        displayName = child->canonicalAlias();
        if (!displayName.isEmpty()) {
            return displayName;
        }

        return child->id();
    }
    if (role == AvatarUrlRole) {
        return child->avatarUrl();
    }
    if (role == TopicRole) {
        return child->topic();
    }
    if (role == RoomIDRole) {
        return child->id();
    }
    if (role == AliasRole) {
        return child->canonicalAlias();
    }
    if (role == MemberCountRole) {
        return child->memberCount();
    }
    if (role == AllowGuestsRole) {
        return child->allowGuests();
    }
    if (role == WorldReadableRole) {
        return child->worldReadable();
    }
    if (role == IsJoinedRole) {
        return child->isJoined();
    }
    if (role == IsSpaceRole) {
        return child->isSpace();
    }
    if (role == IsSuggestedRole) {
        return child->isSuggested();
    }
    if (role == CanAddChildrenRole) {
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(child->id()))) {
            return room->canSendState(QLatin1String("m.space.child"));
        }
        return false;
    }
    if (role == ParentDisplayNameRole) {
        const auto parent = child->parentItem();
        auto displayName = parent->name();
        if (!displayName.isEmpty()) {
            return displayName;
        }

        displayName = parent->canonicalAlias();
        if (!displayName.isEmpty()) {
            return displayName;
        }

        return parent->id();
    }
    if (role == CanSetParentRole) {
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(child->id()))) {
            return room->canSendState(QLatin1String("m.space.parent"));
        }
        return false;
    }
    if (role == IsDeclaredParentRole) {
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(child->id()))) {
            return room->currentState().contains(QLatin1String("m.space.parent"), child->parentItem()->id());
        }
        return false;
    }
    if (role == CanRemove) {
        const auto parent = child->parentItem();
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(parent->id()))) {
            return room->canSendState(QLatin1String("m.space.child"));
        }
        return false;
    }
    if (role == ParentRoomRole) {
        if (const auto parentRoom = static_cast<NeoChatRoom *>(m_space->connection()->room(child->parentItem()->id()))) {
            return QVariant::fromValue(parentRoom);
        }
        return QVariant::fromValue(nullptr);
    }

    return {};
}

QModelIndex SpaceChildrenModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    SpaceTreeItem *parentItem = getItem(parent);
    if (!parentItem) {
        return QModelIndex();
    }

    SpaceTreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex SpaceChildrenModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    SpaceTreeItem *childItem = static_cast<SpaceTreeItem *>(index.internalPointer());
    SpaceTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int SpaceChildrenModel::rowCount(const QModelIndex &parent) const
{
    SpaceTreeItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<SpaceTreeItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int SpaceChildrenModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QHash<int, QByteArray> SpaceChildrenModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "displayName";
    roles[AvatarUrlRole] = "avatarUrl";
    roles[TopicRole] = "topic";
    roles[RoomIDRole] = "roomId";
    roles[MemberCountRole] = "memberCount";
    roles[AllowGuestsRole] = "allowGuests";
    roles[WorldReadableRole] = "worldReadable";
    roles[IsJoinedRole] = "isJoined";
    roles[AliasRole] = "alias";
    roles[IsSpaceRole] = "isSpace";
    roles[IsSuggestedRole] = "isSuggested";
    roles[CanAddChildrenRole] = "canAddChildren";
    roles[ParentDisplayNameRole] = "parentDisplayName";
    roles[CanSetParentRole] = "canSetParent";
    roles[IsDeclaredParentRole] = "isDeclaredParent";
    roles[CanRemove] = "canRemove";
    roles[ParentRoomRole] = "parentRoom";

    return roles;
}

bool SpaceChildrenModel::isRoomReplaced(const QString &roomId) const
{
    return m_replacedRooms.contains(roomId);
}

void SpaceChildrenModel::addPendingChild(const QString &childName)
{
    m_pendingChildren += childName;
}

#include "moc_spacechildrenmodel.cpp"
