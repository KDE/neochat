// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sortfilterroomtreemodel.h"

#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroomtype.h"
#include "roomtreemodel.h"
#include "spacehierarchycache.h"

SortFilterRoomTreeModel::SortFilterRoomTreeModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
    sort(0);
    invalidateFilter();
    connect(this, &SortFilterRoomTreeModel::filterTextChanged, this, &SortFilterRoomTreeModel::invalidateFilter);
    connect(this, &SortFilterRoomTreeModel::sourceModelChanged, this, [this]() {
        sourceModel()->disconnect(this);
        connect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &SortFilterRoomTreeModel::invalidateFilter);
        connect(sourceModel(), &QAbstractItemModel::rowsRemoved, this, &SortFilterRoomTreeModel::invalidateFilter);
    });

    connect(NeoChatConfig::self(), &NeoChatConfig::CollapsedChanged, this, &SortFilterRoomTreeModel::invalidateFilter);
}

void SortFilterRoomTreeModel::setRoomSortOrder(SortFilterRoomTreeModel::RoomSortOrder sortOrder)
{
    m_sortOrder = sortOrder;
    Q_EMIT roomSortOrderChanged();
    if (sortOrder == SortFilterRoomTreeModel::Alphabetical) {
        setSortRole(RoomTreeModel::DisplayNameRole);
    } else if (sortOrder == SortFilterRoomTreeModel::LastActivity) {
        setSortRole(RoomTreeModel::LastActiveTimeRole);
    }
    invalidate();
}

SortFilterRoomTreeModel::RoomSortOrder SortFilterRoomTreeModel::roomSortOrder() const
{
    return m_sortOrder;
}

bool SortFilterRoomTreeModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_sortOrder == SortFilterRoomTreeModel::LastActivity) {
        // display favorite rooms always on top
        const auto categoryLeft = static_cast<NeoChatRoomType::Types>(sourceModel()->data(source_left, RoomTreeModel::CategoryRole).toInt());
        const auto categoryRight = static_cast<NeoChatRoomType::Types>(sourceModel()->data(source_right, RoomTreeModel::CategoryRole).toInt());

        if (categoryLeft == NeoChatRoomType::Types::Favorite && categoryRight == NeoChatRoomType::Types::Favorite) {
            return sourceModel()->data(source_left, RoomTreeModel::LastActiveTimeRole).toDateTime()
                > sourceModel()->data(source_right, RoomTreeModel::LastActiveTimeRole).toDateTime();
        }
        if (categoryLeft == NeoChatRoomType::Types::Favorite) {
            return true;
        } else if (categoryRight == NeoChatRoomType::Types::Favorite) {
            return false;
        }

        return sourceModel()->data(source_left, RoomTreeModel::LastActiveTimeRole).toDateTime()
            > sourceModel()->data(source_right, RoomTreeModel::LastActiveTimeRole).toDateTime();
    }
    if (m_sortOrder != SortFilterRoomTreeModel::Categories) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
    if (sourceModel()->data(source_left, RoomTreeModel::CategoryRole) != sourceModel()->data(source_right, RoomTreeModel::CategoryRole)) {
        return sourceModel()->data(source_left, RoomTreeModel::CategoryRole).toInt() < sourceModel()->data(source_right, RoomTreeModel::CategoryRole).toInt();
    }
    return sourceModel()->data(source_left, RoomTreeModel::LastActiveTimeRole).toDateTime()
        > sourceModel()->data(source_right, RoomTreeModel::LastActiveTimeRole).toDateTime();
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
        if (sourceModel()->data(sourceModel()->index(source_row, 0), RoomTreeModel::CategoryRole).toInt() == NeoChatRoomType::Search
            && NeoChatConfig::collapsed()) {
            return true;
        }
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

    if (sourceModel()->data(index, RoomTreeModel::JoinStateRole).toString() == QStringLiteral("upgraded")
        && dynamic_cast<RoomTreeModel *>(sourceModel())->connection()->room(sourceModel()->data(index, RoomTreeModel::ReplacementIdRole).toString())) {
        return false;
    }

    if (m_activeSpaceId.isEmpty()) {
        if (!SpaceHierarchyCache::instance().isChild(sourceModel()->data(index, RoomTreeModel::RoomIdRole).toString())) {
            return acceptRoom;
        }
        return false;
    } else {
        const auto &rooms = SpaceHierarchyCache::instance().getRoomListForSpace(m_activeSpaceId, false);
        return std::find(rooms.begin(), rooms.end(), sourceModel()->data(index, RoomTreeModel::RoomIdRole).toString()) != rooms.end() && acceptRoom;
    }
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

#include "moc_sortfilterroomtreemodel.cpp"
