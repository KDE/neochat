// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "spacechildrenmodel.h"

#include <Quotient/jobs/basejob.h>
#include <Quotient/room.h>

#include "neochatconnection.h"

SpaceChildrenModel::SpaceChildrenModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new SpaceTreeItem(nullptr))
{
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
        m_space->connection()->disconnect(this);
        m_space->disconnect(this);
    }

    m_space = space;
    Q_EMIT spaceChanged();

    refreshModel();

    if (!m_space) {
        return;
    }

    connect(m_space->connection(), &NeoChatConnection::loadedRoomState, this, [this](Quotient::Room *room) {
        if (m_pendingChildren.contains(room->name())) {
            m_pendingChildren.removeAll(room->name());
            refreshModel();
        }
    });
    connect(m_space, &Quotient::Room::changed, this, &SpaceChildrenModel::refreshModel);
}

bool SpaceChildrenModel::loading() const
{
    return m_loading;
}

void SpaceChildrenModel::refreshModel()
{
    for (const auto &job : m_currentJobs) {
        if (job) {
            job->abandon();
        }
    }
    m_currentJobs.clear();

    if (m_space == nullptr) {
        beginResetModel();
        delete m_rootItem;
        m_rootItem = nullptr;
        endResetModel();
        return;
    }

    beginResetModel();
    m_replacedRooms.clear();
    delete m_rootItem;
    m_loading = true;
    Q_EMIT loadingChanged();
    m_rootItem =
        new SpaceTreeItem(dynamic_cast<NeoChatConnection *>(m_space->connection()), nullptr, m_space->id(), m_space->displayName(), m_space->canonicalAlias());
    endResetModel();
    m_currentJobs.append(
        m_space->connection()->callApi<Quotient::GetSpaceHierarchyJob>(m_space->id(), std::nullopt, std::nullopt, 1).then([this](const auto &job) {
            insertChildren(job->rooms());
        }));
}

void SpaceChildrenModel::insertChildren(std::vector<Quotient::GetSpaceHierarchyJob::SpaceHierarchyRoomsChunk> children, const QModelIndex &parent)
{
    const auto parentItem = getItem(parent);

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
    for (auto &child : children) {
        if (child.roomId == m_space->id() || child.roomId == parentItem->id()) {
            continue;
        } else {
            int insertRow = parentItem->childCount();
            if (const auto room = m_space->connection()->room(child.roomId)) {
                const auto predecessorId = room->predecessorId();
                if (!predecessorId.isEmpty()) {
                    m_replacedRooms += predecessorId;
                }
                const auto successorId = room->successorId();
                if (!successorId.isEmpty()) {
                    m_replacedRooms += successorId;
                }
                if (dynamic_cast<NeoChatRoom *>(room)->isSpace()) {
                    connect(room, &Quotient::Room::changed, this, &SpaceChildrenModel::refreshModel);
                }
            }
            if (child.childrenState.size() > 0) {
                m_currentJobs.append(m_space->connection()
                                         ->callApi<Quotient::GetSpaceHierarchyJob>(child.roomId, std::nullopt, std::nullopt, 1)
                                         .then([this, parent, insertRow](const auto &job) {
                                             insertChildren(job->rooms(), index(insertRow, 0, parent));
                                         }));
            }

            parentItem->insertChild(std::make_unique<SpaceTreeItem>(dynamic_cast<NeoChatConnection *>(m_space->connection()),
                                                                    parentItem,
                                                                    child.roomId,
                                                                    child.name,
                                                                    child.canonicalAlias,
                                                                    child.topic,
                                                                    child.numJoinedMembers,
                                                                    child.avatarUrl,
                                                                    child.guestCanJoin,
                                                                    child.worldReadable,
                                                                    child.roomType == u"m.space"_s,
                                                                    std::move(child.childrenState)));
        }
    }
    endInsertRows();
}

SpaceTreeItem *SpaceChildrenModel::getItem(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_rootItem;
    }
    return static_cast<SpaceTreeItem *>(index.internalPointer());
}

QVariant SpaceChildrenModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &child = getItem(index);
    if (role == DisplayNameRole) {
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(child->id()))) {
            return room->displayName();
        }
        if (const auto &displayName = child->name(); !displayName.isEmpty()) {
            return displayName;
        }
        if (const auto &displayName = child->canonicalAlias(); !displayName.isEmpty()) {
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
            return room->canSendState(u"m.space.child"_s);
        }
        return false;
    }
    if (role == ParentDisplayNameRole) {
        const auto parent = child->parentItem();

        if (const auto displayName = parent->name(); !displayName.isEmpty()) {
            return displayName;
        }

        if (const auto displayName = parent->canonicalAlias(); !displayName.isEmpty()) {
            return displayName;
        }

        return parent->id();
    }
    if (role == CanSetParentRole) {
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(child->id()))) {
            return room->canSendState(u"m.space.parent"_s);
        }
        return false;
    }
    if (role == IsDeclaredParentRole) {
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(child->id()))) {
            return room->currentState().contains(u"m.space.parent"_s, child->parentItem()->id());
        }
        return false;
    }
    if (role == CanRemove) {
        const auto parent = child->parentItem();
        if (const auto room = static_cast<NeoChatRoom *>(m_space->connection()->room(parent->id()))) {
            return room->canSendState(u"m.space.child"_s);
        }
        return false;
    }
    if (role == ParentRoomRole) {
        if (const auto parentRoom = static_cast<NeoChatRoom *>(m_space->connection()->room(child->parentItem()->id()))) {
            return QVariant::fromValue(parentRoom);
        }
        return QVariant::fromValue(nullptr);
    }
    if (role == OrderRole) {
        if (child->parentItem() == nullptr) {
            return QString();
        }
        const auto childState = child->parentItem()->childStateContent(child);
        return childState["order"_L1].toString();
    }
    if (role == ChildTimestampRole) {
        if (child->parentItem() == nullptr) {
            return QString();
        }
        const auto childState = child->parentItem()->childState(child);
        return childState["origin_server_ts"_L1].toString();
    }

    return {};
}

QModelIndex SpaceChildrenModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    const auto parentItem = getItem(parent);
    if (!parentItem) {
        return {};
    }

    if (const auto childItem = parentItem->child(row)) {
        return createIndex(row, column, childItem);
    }
    return {};
}

QModelIndex SpaceChildrenModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }

    SpaceTreeItem *childItem = static_cast<SpaceTreeItem *>(index.internalPointer());
    SpaceTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return {};
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
    return {{
        {DisplayNameRole, "displayName"},
        {AvatarUrlRole, "avatarUrl"},
        {TopicRole, "topic"},
        {RoomIDRole, "roomId"},
        {MemberCountRole, "memberCount"},
        {AllowGuestsRole, "allowGuests"},
        {WorldReadableRole, "worldReadable"},
        {IsJoinedRole, "isJoined"},
        {AliasRole, "alias"},
        {IsSpaceRole, "isSpace"},
        {IsSuggestedRole, "isSuggested"},
        {CanAddChildrenRole, "canAddChildren"},
        {ParentDisplayNameRole, "parentDisplayName"},
        {CanSetParentRole, "canSetParent"},
        {IsDeclaredParentRole, "isDeclaredParent"},
        {CanRemove, "canRemove"},
        {ParentRoomRole, "parentRoom"},
        {OrderRole, "order"},
        {ChildTimestampRole, "childTimestamp"},
    }};
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
