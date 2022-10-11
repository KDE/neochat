// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sortfilterroomlistmodel.h"

#include "roomlistmodel.h"
#include "spacehierarchycache.h"

SortFilterRoomListModel::SortFilterRoomListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    sort(0);
    invalidateFilter();
    connect(this, &SortFilterRoomListModel::filterTextChanged, this, [this]() {
        invalidateFilter();
    });
}

void SortFilterRoomListModel::setRoomSortOrder(SortFilterRoomListModel::RoomSortOrder sortOrder)
{
    m_sortOrder = sortOrder;
    Q_EMIT roomSortOrderChanged();
    if (sortOrder == SortFilterRoomListModel::Alphabetical) {
        setSortRole(RoomListModel::NameRole);
    } else if (sortOrder == SortFilterRoomListModel::LastActivity) {
        setSortRole(RoomListModel::LastActiveTimeRole);
    }
    invalidate();
}

SortFilterRoomListModel::RoomSortOrder SortFilterRoomListModel::roomSortOrder() const
{
    return m_sortOrder;
}

static const QVector<RoomListModel::EventRoles> categorySortPriorities{
  // Favorites at the top
  RoomListModel::FavoritedRole,
  // Group by categories
  RoomListModel::CategoryRole,
  // Within each category, pull relevant ones to the top
  RoomListModel::AttentionRole,
  RoomListModel::HighlightCountRole,
  RoomListModel::NotificationCountRole,
  RoomListModel::UnreadCountRole,
  // Finally sort by last activity time
  RoomListModel::LastActiveTimeRole
};

static const QVector<RoomListModel::EventRoles> alphabeticalSortPriorities {
  // Does exactly what it says on the tin.
  (RoomListModel::EventRoles)Qt::DisplayRole
};

static const QVector<RoomListModel::EventRoles> activitySortPriorities{
  // Anything useful at the top, quiet rooms at the bottom
  RoomListModel::AttentionRole,
  // Organize by highlights, notifications, unread favorites, all other unread, in that order
  RoomListModel::HighlightCountRole,
  RoomListModel::NotificationCountRole,
  RoomListModel::FavoritedRole,
  RoomListModel::UnreadCountRole,
  // Finally sort by last activity time
  RoomListModel::LastActiveTimeRole
};

bool SortFilterRoomListModel::roleCmp(RoomListModel::EventRoles role, const QVariant &sortLeft, const QVariant &sortRight) const
{
    switch(role) {
        case RoomListModel::FavoritedRole:
            return (sortLeft == sortRight) ? false : sortLeft.toBool();
        case RoomListModel::CategoryRole:
            return sortLeft < sortRight;
        default:
            return sortLeft > sortRight;
    }
}

bool SortFilterRoomListModel::prioritiesCmp(const QVector<RoomListModel::EventRoles>& priorities, const QModelIndex &source_left, const QModelIndex &source_right) const
{
    for(RoomListModel::EventRoles sortRole : priorities) {
        const auto sortLeft = sourceModel()->data(source_left, sortRole);
        const auto sortRight = sourceModel()->data(source_right, sortRole);
        if (sortLeft != sortRight) {
            return roleCmp(sortRole, sortLeft, sortRight);
        }
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

bool SortFilterRoomListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    switch(m_sortOrder) {
        case SortFilterRoomListModel::Alphabetical:
          return prioritiesCmp(alphabeticalSortPriorities, source_left, source_right);
        case SortFilterRoomListModel::Categories:
          return prioritiesCmp(categorySortPriorities, source_left, source_right);
        case SortFilterRoomListModel::LastActivity:
          return prioritiesCmp(activitySortPriorities, source_left, source_right);
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

void SortFilterRoomListModel::setFilterText(const QString &text)
{
    m_filterText = text;
    Q_EMIT filterTextChanged();
}

QString SortFilterRoomListModel::filterText() const
{
    return m_filterText;
}

bool SortFilterRoomListModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);

    bool acceptRoom = sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::NameRole).toString().contains(m_filterText, Qt::CaseInsensitive)
        && sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::JoinStateRole).toString() != "upgraded"
        && sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IsSpaceRole).toBool() == false;

    if (m_activeSpaceId.isEmpty()) {
        return acceptRoom;
    } else {
        const auto &rooms = SpaceHierarchyCache::instance().getRoomListForSpace(m_activeSpaceId, false);
        return std::find(rooms.begin(), rooms.end(), sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IdRole).toString()) != rooms.end()
            && acceptRoom;
    }
}

QString SortFilterRoomListModel::activeSpaceId() const
{
    return m_activeSpaceId;
}

void SortFilterRoomListModel::setActiveSpaceId(const QString &spaceId)
{
    m_activeSpaceId = spaceId;
    Q_EMIT activeSpaceIdChanged();
    invalidate();
}
