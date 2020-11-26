/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sortfilterroomlistmodel.h"

#include "roomlistmodel.h"

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

bool SortFilterRoomListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_sortOrder == SortFilterRoomListModel::LastActivity) {
        // display favorite rooms always on top
        const auto categoryLeft = static_cast<RoomType::Types>(sourceModel()->data(source_left, RoomListModel::CategoryRole).toInt());
        const auto categoryRight = static_cast<RoomType::Types>(sourceModel()->data(source_right, RoomListModel::CategoryRole).toInt());

        if (categoryLeft == RoomType::Types::Favorite && categoryRight == RoomType::Types::Favorite) {
            return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole).toDateTime() > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole).toDateTime();
        }
        if (categoryLeft == RoomType::Types::Favorite) {
            return true;
        } else if (categoryRight == RoomType::Types::Favorite) {
            return false;
        }

        return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole).toDateTime() > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole).toDateTime();
    }
    if (m_sortOrder != SortFilterRoomListModel::Categories) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
    if (sourceModel()->data(source_left, RoomListModel::CategoryRole) != sourceModel()->data(source_right, RoomListModel::CategoryRole)) {
        return sourceModel()->data(source_left, RoomListModel::CategoryRole).toInt() < sourceModel()->data(source_right, RoomListModel::CategoryRole).toInt();
    }
    return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole).toDateTime() > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole).toDateTime();
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
    return sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::NameRole).toString().contains(m_filterText, Qt::CaseInsensitive) &&
        sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::JoinStateRole).toString() != "upgraded";
}
