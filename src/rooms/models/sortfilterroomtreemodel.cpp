// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sortfilterroomtreemodel.h"

#include "enums/neochatroomtype.h"
#include "enums/roomsortparameter.h"
#include "models/roomtreemodel.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "spacehierarchycache.h"

#include <Quotient/eventstats.h>

bool SortFilterRoomTreeModel::m_showAllRoomsInHome = false;

SortFilterRoomTreeModel::SortFilterRoomTreeModel(RoomTreeModel *sourceModel, QObject *parent)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceModel);
    setSourceModel(sourceModel);

    setRecursiveFilteringEnabled(true);
    sort(0);
    connect(this, &SortFilterRoomTreeModel::filterTextChanged, this, &SortFilterRoomTreeModel::invalidateFilter);
    connect(sourceModel, &RoomTreeModel::invalidateSort, this, &SortFilterRoomTreeModel::invalidate);
    connect(this, &SortFilterRoomTreeModel::sourceModelChanged, this, [this]() {
        this->sourceModel()->disconnect(this);
        connect(this->sourceModel(), &QAbstractItemModel::rowsInserted, this, &SortFilterRoomTreeModel::invalidateFilter);
        connect(this->sourceModel(), &QAbstractItemModel::rowsRemoved, this, &SortFilterRoomTreeModel::invalidateFilter);
    });
}

bool SortFilterRoomTreeModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto treeModel = dynamic_cast<RoomTreeModel *>(sourceModel());
    if (treeModel == nullptr) {
        return false;
    }

    // Don't sort the top level categories, unless there's a server notice with unread messages.
    if (!source_left.parent().isValid() || !source_right.parent().isValid()) {
        if (source_left.row() == NeoChatRoomType::ServerNotice) {
            for (int i = 0; i < treeModel->rowCount(source_left); i++) {
                auto room = treeModel->connection()->room(treeModel->index(i, 0, source_left).data(RoomTreeModel::RoomIdRole).toString());
                if (room && room->unreadStats().notableCount > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    const auto leftRoom = dynamic_cast<NeoChatRoom *>(treeModel->connection()->room(source_left.data(RoomTreeModel::RoomIdRole).toString()));
    const auto rightRoom = dynamic_cast<NeoChatRoom *>(treeModel->connection()->room(source_right.data(RoomTreeModel::RoomIdRole).toString()));
    if (leftRoom == nullptr || rightRoom == nullptr) {
        return false;
    }

    for (auto sortRole : RoomSortParameter::currentParameterList()) {
        auto result = RoomSortParameter::compareParameter(sortRole, leftRoom, rightRoom);

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

    if (m_showAllRoomsInHome && m_activeSpaceId.isEmpty()) {
        return acceptRoom;
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

void SortFilterRoomTreeModel::setCurrentRoom(NeoChatRoom *room)
{
    m_currentRoom = room;
    Q_EMIT currentRoomChanged();
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

    return mapFromSource(roomModel->indexForRoom(m_currentRoom));
}

void SortFilterRoomTreeModel::setShowAllRoomsInHome(bool enabled)
{
    SortFilterRoomTreeModel::m_showAllRoomsInHome = enabled;
}

#include "moc_sortfilterroomtreemodel.cpp"
