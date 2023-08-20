// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
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
        setSortRole(RoomListModel::DisplayNameRole);
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
        const auto categoryLeft = static_cast<NeoChatRoomType::Types>(sourceModel()->data(source_left, RoomListModel::CategoryRole).toInt());
        const auto categoryRight = static_cast<NeoChatRoomType::Types>(sourceModel()->data(source_right, RoomListModel::CategoryRole).toInt());

        if (categoryLeft == NeoChatRoomType::Types::Favorite && categoryRight == NeoChatRoomType::Types::Favorite) {
            return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole).toDateTime()
                > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole).toDateTime();
        }
        if (categoryLeft == NeoChatRoomType::Types::Favorite) {
            return true;
        } else if (categoryRight == NeoChatRoomType::Types::Favorite) {
            return false;
        }

        return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole).toDateTime()
            > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole).toDateTime();
    }
    if (m_sortOrder != SortFilterRoomListModel::Categories) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
    if (sourceModel()->data(source_left, RoomListModel::CategoryRole) != sourceModel()->data(source_right, RoomListModel::CategoryRole)) {
        return sourceModel()->data(source_left, RoomListModel::CategoryRole).toInt() < sourceModel()->data(source_right, RoomListModel::CategoryRole).toInt();
    }
    return sourceModel()->data(source_left, RoomListModel::LastActiveTimeRole).toDateTime()
        > sourceModel()->data(source_right, RoomListModel::LastActiveTimeRole).toDateTime();
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

    bool acceptRoom =
        sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::DisplayNameRole).toString().contains(m_filterText, Qt::CaseInsensitive)
        && sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::JoinStateRole).toString() != QStringLiteral("upgraded")
        && sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IsSpaceRole).toBool() == false;

    if (m_activeSpaceId.isEmpty()) {
        return acceptRoom;
    } else {
        const auto &rooms = SpaceHierarchyCache::instance().getRoomListForSpace(m_activeSpaceId, false);
        return std::find(rooms.begin(), rooms.end(), sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::RoomIdRole).toString())
            != rooms.end()
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

#include "moc_sortfilterroomlistmodel.cpp"
