// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "timelinemodel.h"

#include <Quotient/qt_connection_util.h>

#include "delegatetype.h"

TimelineModel::TimelineModel(QObject *parent)
    : QConcatenateTablesProxyModel(parent)
{
    m_timelineBeginningModel = new TimelineBeginningModel(this);
    addSourceModel(m_timelineBeginningModel);
    m_messageEventModel = new MessageEventModel(this);
    addSourceModel(m_messageEventModel);
    m_timelineEndModel = new TimelineEndModel(this);
    addSourceModel(m_timelineEndModel);
}

NeoChatRoom *TimelineModel::room() const
{
    return m_messageEventModel->room();
}

void TimelineModel::setRoom(NeoChatRoom *room)
{
    // Both models do their own null checking so just pass along.
    m_messageEventModel->setRoom(room);
    m_timelineBeginningModel->setRoom(room);
    m_timelineEndModel->setRoom(room);
}

MessageEventModel *TimelineModel::messageEventModel() const
{
    return m_messageEventModel;
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    return m_messageEventModel->roleNames();
}

TimelineBeginningModel::TimelineBeginningModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void TimelineBeginningModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }

    beginResetModel();

    if (m_room != nullptr) {
        m_room->disconnect(this);
    }

    m_room = room;

    if (m_room != nullptr) {
        Quotient::connectUntil(m_room.get(), &Quotient::Room::eventsHistoryJobChanged, this, [this]() {
            if (m_room && m_room->allHistoryLoaded()) {
                // HACK: We have to do it this way because DelegateChooser doesn't update dynamically.
                beginRemoveRows({}, 0, 0);
                endRemoveRows();
                beginInsertRows({}, 0, 0);
                endInsertRows();
                return true;
            }
            return false;
        });
    }

    endResetModel();
}

QVariant TimelineBeginningModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(idx)
    if (m_room == nullptr) {
        return {};
    }

    if (role == DelegateTypeRole) {
        return DelegateType::Successor;
    }
    return {};
}

int TimelineBeginningModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_room == nullptr) {
        return 1;
    }
    return m_room->successorId().isEmpty() ? 0 : 1;
}

QHash<int, QByteArray> TimelineBeginningModel::roleNames() const
{
    return {{DelegateTypeRole, "delegateType"}};
}

TimelineEndModel::TimelineEndModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void TimelineEndModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }

    beginResetModel();

    if (m_room != nullptr) {
        m_room->disconnect(this);
    }

    m_room = room;

    if (m_room != nullptr) {
        connect(m_room, &Quotient::Room::eventsHistoryJobChanged, this, [this]() {
            if (m_room->allHistoryLoaded()) {
                // HACK: We have to do it this way because DelegateChooser doesn't update dynamically.
                beginRemoveRows({}, 0, 0);
                endRemoveRows();
                beginInsertRows({}, 0, 0);
                endInsertRows();
            }
        });
    }

    endResetModel();
}

QVariant TimelineEndModel::data(const QModelIndex &idx, int role) const
{
    Q_UNUSED(idx)
    if (m_room == nullptr) {
        return {};
    }

    if (role == DelegateTypeRole) {
        if (idx.row() == 1 || rowCount() == 1) {
            return m_room->allHistoryLoaded() ? DelegateType::TimelineEnd : DelegateType::Loading;
        } else {
            return DelegateType::Predecessor;
        }
    }
    return {};
}

int TimelineEndModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_room == nullptr) {
        return 1;
    }
    return m_room->predecessorId().isEmpty() ? 1 : (m_room->allHistoryLoaded() ? 2 : 1);
}

QHash<int, QByteArray> TimelineEndModel::roleNames() const
{
    return {{DelegateTypeRole, "delegateType"}};
}

#include "moc_timelinemodel.cpp"
