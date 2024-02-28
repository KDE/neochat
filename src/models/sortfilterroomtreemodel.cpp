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
    setRoomSortOrder(static_cast<RoomSortOrder>(NeoChatConfig::sortOrder()));
    connect(NeoChatConfig::self(), &NeoChatConfig::SortOrderChanged, this, [this]() {
        setRoomSortOrder(static_cast<RoomSortOrder>(NeoChatConfig::sortOrder()));
        invalidateFilter();
    });

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
    if (sortOrder == SortFilterRoomTreeModel::Alphabetical) {
        setSortRole(RoomTreeModel::DisplayNameRole);
    } else if (sortOrder == SortFilterRoomTreeModel::Activity) {
        setSortRole(RoomTreeModel::LastActiveTimeRole);
    }
    invalidate();
}

static const QVector<RoomTreeModel::EventRoles> alphabeticalSortPriorities{
    // Does exactly what it says on the tin.
    RoomTreeModel::DisplayNameRole,
};

static const QVector<RoomTreeModel::EventRoles> activitySortPriorities{
    // Anything useful at the top, quiet rooms at the bottom
    RoomTreeModel::AttentionRole,
    // Organize by highlights, notifications, unread favorites, all other unread, in that order
    RoomTreeModel::HighlightCountRole,
    RoomTreeModel::NotificationCountRole,
    RoomTreeModel::FavouriteRole,
    // Finally sort by last activity time
    RoomTreeModel::LastActiveTimeRole,
};

bool SortFilterRoomTreeModel::roleCmp(const QVariant &sortLeft, const QVariant &sortRight) const
{
    switch (sortLeft.typeId()) {
    case QMetaType::Bool:
        return (sortLeft == sortRight) ? false : sortLeft.toBool();
    case QMetaType::QString:
        return sortLeft.toString() < sortRight.toString();
    case QMetaType::Int:
        return sortLeft.toInt() > sortRight.toInt();
    case QMetaType::QDateTime:
        return sortLeft.toDateTime() > sortRight.toDateTime();
    default:
        return false;
    }
}

bool SortFilterRoomTreeModel::prioritiesCmp(const QVector<RoomTreeModel::EventRoles> &priorities,
                                            const QModelIndex &source_left,
                                            const QModelIndex &source_right) const
{
    for (RoomTreeModel::EventRoles sortRole : priorities) {
        const auto sortLeft = sourceModel()->data(source_left, sortRole);
        const auto sortRight = sourceModel()->data(source_right, sortRole);
        if (sortLeft != sortRight) {
            return roleCmp(sortLeft, sortRight);
        }
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

bool SortFilterRoomTreeModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    // Don't sort the top level categories.
    if (!source_left.parent().isValid() || !source_right.parent().isValid()) {
        return false;
    }

    switch (m_sortOrder) {
    case SortFilterRoomTreeModel::Alphabetical:
        return prioritiesCmp(alphabeticalSortPriorities, source_left, source_right);
    case SortFilterRoomTreeModel::Activity:
        return prioritiesCmp(activitySortPriorities, source_left, source_right);
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
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
