// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "statekeysmodel.h"

StateKeysModel::StateKeysModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> StateKeysModel::roleNames() const
{
    return {
        {StateKeyRole, "stateKey"},
    };
}
QVariant StateKeysModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
    const auto row = index.row();
    switch (role) {
    case StateKeyRole:
        return m_stateKeys[row]->stateKey();
    }
    return {};
}

int StateKeysModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_stateKeys.count();
}

NeoChatRoom *StateKeysModel::room() const
{
    return m_room;
}

void StateKeysModel::loadState()
{
    if (!m_room || m_eventType.isEmpty()) {
        return;
    }

    beginResetModel();
    m_stateKeys = m_room->currentState().eventsOfType(m_eventType);
    endResetModel();
}

void StateKeysModel::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
    }

    m_room = room;
    Q_EMIT roomChanged();

    if (room) {
        loadState();

        connect(room, &NeoChatRoom::changed, this, [this] {
            loadState();
        });
    }
}

QString StateKeysModel::eventType() const
{
    return m_eventType;
}

void StateKeysModel::setEventType(const QString &eventType)
{
    m_eventType = eventType;
    Q_EMIT eventTypeChanged();
    loadState();
}

QByteArray StateKeysModel::stateEventJson(const QModelIndex &index)
{
    const auto row = index.row();
    const auto event = m_stateKeys[row];
    const auto json = event->fullJson();
    return QJsonDocument(json).toJson();
}

#include "moc_statekeysmodel.cpp"
