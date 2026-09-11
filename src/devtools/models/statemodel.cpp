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
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        qWarning() << Q_FUNC_INFO << "called with invalid index" << index << role;
        return {};
    }

    auto row = index.row();
    switch (role) {
    case TypeRole:
        return m_stateEvents.keys()[row];
    case EventCountRole:
        return m_stateEvents.values()[row].count();
    case StateKeyRole:
        if (m_stateEvents.values()[row].isEmpty()) {
            qWarning() << Q_FUNC_INFO << "no state keys for" << m_stateEvents.keys()[row];
            return {};
        }
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
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
    }

    m_room = room;
    Q_EMIT roomChanged();
    loadState();

    connect(room, &NeoChatRoom::changed, this, &StateModel::loadState);
}

QByteArray StateModel::stateEventJson(const QString &type, const QString &stateKey)
{
    const auto state = m_room->currentState().get(type, stateKey);
    if (!state) {
        qWarning() << Q_FUNC_INFO << "state" << type << stateKey << "not found";
        return {};
    }
    return QJsonDocument(state->fullJson()).toJson();
}

QByteArray StateModel::stateEventContentJson(const QString &type, const QString &stateKey)
{
    const auto state = m_room->currentState().get(type, stateKey);
    if (!state) {
        qWarning() << Q_FUNC_INFO << "state" << type << stateKey << "not found";
        return {};
    }
    return QJsonDocument(state->contentJson()).toJson();
}

#include "moc_statemodel.cpp"
