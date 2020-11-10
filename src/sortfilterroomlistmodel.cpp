/**
 * SPDX-FileCopyrightText: Tobias Fella <fella@posteo.de>
 *
 * SPDX-LicenseIdentifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "sortfilterroomlistmodel.h"

#include "roomlistmodel.h"

SortFilterRoomListModel::SortFilterRoomListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterRole(RoomListModel::NameRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort(0);
}

void SortFilterRoomListModel::setRoomSortOrder(SortFilterRoomListModel::RoomSortOrder sortOrder)
{
    m_sortOrder = sortOrder;
    Q_EMIT roomSortOrderChanged();
    if (sortOrder == SortFilterRoomListModel::Alphabetical)
        setSortRole(RoomListModel::NameRole);
    else if (sortOrder == SortFilterRoomListModel::LastActivity)
        setSortRole(RoomListModel::LastActiveTimeRole);
}

SortFilterRoomListModel::RoomSortOrder SortFilterRoomListModel::roomSortOrder() const
{
    return m_sortOrder;
}

bool SortFilterRoomListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_sortOrder != SortFilterRoomListModel::Categories)
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    if (sourceModel()->data(source_left, RoomListModel::CategoryRole) != sourceModel()->data(source_right, RoomListModel::CategoryRole))
        return sourceModel()->data(source_left, RoomListModel::CategoryRole) < sourceModel()->data(source_right, RoomListModel::CategoryRole);
    return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole) > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole);
}

void SortFilterRoomListModel::setFilterText(const QString &text)
{
    setFilterFixedString(text);
}
