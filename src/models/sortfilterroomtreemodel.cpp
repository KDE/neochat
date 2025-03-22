// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sortfilterroomtreemodel.h"

#include "roomsortparameter.h"
// #include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroomtype.h"
#include <Integral/Room>
// #include "roommanager.h"
#include "roomtreemodel.h"
// #include "spacehierarchycache.h"

SortFilterRoomTreeModel::SortFilterRoomTreeModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Q_ASSERT(sourceModel);
    // setSourceModel(sourceModel);

    // setRoomSortOrder(static_cast<RoomSortOrder>(NeoChatConfig::sortOrder()));
    // connect(NeoChatConfig::self(), &NeoChatConfig::SortOrderChanged, this, [this]() {
    //     setRoomSortOrder(static_cast<RoomSortOrder>(NeoChatConfig::sortOrder()));
    //     invalidateFilter();
    // });

    setRecursiveFilteringEnabled(true);
    sort(0);
    connect(this, &SortFilterRoomTreeModel::filterTextChanged, this, &SortFilterRoomTreeModel::invalidateFilter);
    connect(this, &SortFilterRoomTreeModel::sourceModelChanged, this, [this]() {
        this->sourceModel()->disconnect(this);
        connect(this->sourceModel(), &QAbstractItemModel::rowsInserted, this, &SortFilterRoomTreeModel::invalidateFilter);
        connect(this->sourceModel(), &QAbstractItemModel::rowsRemoved, this, &SortFilterRoomTreeModel::invalidateFilter);
    });

    // connect(NeoChatConfig::self(), &NeoChatConfig::CollapsedChanged, this, &SortFilterRoomTreeModel::invalidateFilter);
    // connect(NeoChatConfig::self(), &NeoChatConfig::AllRoomsInHomeChanged, this, [this]() {
    //     invalidateFilter();
    //     if (NeoChatConfig::self()->allRoomsInHome()) {
    //         RoomManager::instance().resetState();
    //     }
    // });
}

void SortFilterRoomTreeModel::setRoomSortOrder(SortFilterRoomTreeModel::RoomSortOrder sortOrder)
{
    m_sortOrder = sortOrder;
    invalidate();
}

static const QVector<RoomSortParameter::Parameter> alphabeticalSortPriorities{
    // Does exactly what it says on the tin.
    RoomSortParameter::AlphabeticalAscending,
};

static const QVector<RoomSortParameter::Parameter> activitySortPriorities{
    RoomSortParameter::HasHighlight,
    RoomSortParameter::MostHighlights,
    RoomSortParameter::HasUnread,
    RoomSortParameter::MostUnread,
    RoomSortParameter::LastActive,
};

static const QVector<RoomSortParameter::Parameter> lastMessageSortPriorities{
    RoomSortParameter::LastActive,
};

bool SortFilterRoomTreeModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    // Don't sort the top level categories.
    if (!source_left.parent().isValid() || !source_right.parent().isValid()) {
        return false;
    }

    const auto treeModel = dynamic_cast<RoomTreeModel *>(sourceModel());
    if (treeModel == nullptr) {
        return false;
    }

    const auto leftRoom = treeModel->roomForIndex(source_left);
    const auto rightRoom = treeModel->roomForIndex(source_right);
    if (!leftRoom.has_value() || !rightRoom.has_value()) {
        return false;
    }

    for (auto sortRole : RoomSortParameter::currentParameterList()) {
        auto result = RoomSortParameter::compareParameter(sortRole, leftRoom.value()->box_me(), rightRoom.value()->box_me());

        if (result != 0) {
            return result > 0;
        }
    }
    return false;
}

void SortFilterRoomTreeModel::setFilterText(const QString &text)
{
    m_filterText = text;
    Q_EMIT filterTextChanged();
}

QString SortFilterRoomTreeModel::filterText() const
{
    return m_filterText;
}

bool SortFilterRoomTreeModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!source_parent.isValid()) {
        if (sourceModel()->data(sourceModel()->index(source_row, 0), RoomTreeModel::CategoryRole).toInt() == NeoChatRoomType::AddDirect
            && m_mode == DirectChats) {
            return true;
        }
        return false;
    }

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    bool acceptRoom = sourceModel()->data(index, RoomTreeModel::DisplayNameRole).toString().contains(m_filterText, Qt::CaseInsensitive)
        && sourceModel()->data(index, RoomTreeModel::IsSpaceRole).toBool() == false;

    bool isDirectChat = sourceModel()->data(index, RoomTreeModel::IsDirectChat).toBool();
    // In `show direct chats` mode we only care about whether or not it's a direct chat or if the filter string matches.'
    if (m_mode == DirectChats) {
        return isDirectChat && acceptRoom;
    }

    // When not in `show direct chats` mode, filter them out.
    if (isDirectChat && m_mode == Rooms) {
        return false;
    }

    if (sourceModel()->data(index, RoomTreeModel::JoinStateRole).toString() == u"upgraded"_s
        && dynamic_cast<RoomTreeModel *>(sourceModel())->connection()->room(sourceModel()->data(index, RoomTreeModel::ReplacementIdRole).toString())) {
        return false;
    }

    // Hide rooms with defined types, assuming that data-holding rooms have a defined type
    if (!sourceModel()->data(index, RoomTreeModel::RoomTypeRole).toString().isEmpty()) {
        return false;
    }

    return acceptRoom;

    // static auto config = NeoChatConfig::self();
    // if (config->allRoomsInHome() && RoomManager::instance().currentSpace().isEmpty()) {
    //     return acceptRoom;
    // }
    //
    // if (m_activeSpaceId.isEmpty()) {
    //     if (!SpaceHierarchyCache::instance().isChild(sourceModel()->data(index, RoomTreeModel::RoomIdRole).toString())) {
    //         return acceptRoom;
    //     }
    //     return false;
    // } else {
    //     const auto &rooms = SpaceHierarchyCache::instance().getRoomListForSpace(m_activeSpaceId, false);
    //     return std::find(rooms.begin(), rooms.end(), sourceModel()->data(index, RoomTreeModel::RoomIdRole).toString()) != rooms.end() && acceptRoom;
    // }
}

QString SortFilterRoomTreeModel::activeSpaceId() const
{
    return m_activeSpaceId;
}

void SortFilterRoomTreeModel::setActiveSpaceId(const QString &spaceId)
{
    m_activeSpaceId = spaceId;
    Q_EMIT activeSpaceIdChanged();
    invalidate();
}

SortFilterRoomTreeModel::Mode SortFilterRoomTreeModel::mode() const
{
    return m_mode;
}

void SortFilterRoomTreeModel::setMode(SortFilterRoomTreeModel::Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    Q_EMIT modeChanged();
    invalidate();
}

QModelIndex SortFilterRoomTreeModel::currentRoomIndex() const
{
    const auto roomModel = dynamic_cast<RoomTreeModel *>(sourceModel());
    if (roomModel == nullptr) {
        return {};
    }

    return {}; // mapFromSource(roomModel->indexForRoom(RoomManager::instance().currentRoom()));
}

#include "moc_sortfilterroomtreemodel.cpp"
