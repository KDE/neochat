// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "statemodel.h"

StateModel::StateModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> StateModel::roleNames() const
{
    return {
        {TypeRole, "type"},
        {EventCountRole, "eventCount"},
        {StateKeyRole, "stateKey"},
    };
}
QVariant StateModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    switch (role) {
    case TypeRole:
        return m_stateEvents.keys()[row];
    case EventCountRole:
        return m_stateEvents.values()[row].count();
    case StateKeyRole:
        return m_stateEvents.values()[row][0];
    }
    return {};
}

int StateModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_stateEvents.count();
}

NeoChatRoom *StateModel::room() const
{
    return m_room;
}

void StateModel::loadState()
{
    beginResetModel();
    m_stateEvents.clear();
    if (!m_room) {
        endResetModel();
        return;
    }
    const auto keys = m_room->currentState().events().keys();
    for (const auto &[type, stateKey] : keys) {
        if (!m_stateEvents.contains(type)) {
            m_stateEvents[type] = {};
        }
        m_stateEvents[type] += stateKey;
    }
    endResetModel();
}

void StateModel::setRoom(NeoChatRoom *room)
{
    m_room = room;
    Q_EMIT roomChanged();
    loadState();

    connect(room, &NeoChatRoom::changed, this, [this] {
        loadState();
    });
}

QByteArray StateModel::stateEventJson(const QString &type, const QString &stateKey)
{
    return QJsonDocument(m_room->currentState().get(type, stateKey)->fullJson()).toJson();
}

QByteArray StateModel::stateEventContentJson(const QString &type, const QString &stateKey)
{
    return QJsonDocument(m_room->currentState().get(type, stateKey)->contentJson()).toJson();
}

#include "moc_statemodel.cpp"
