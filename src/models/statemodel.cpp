// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "statemodel.h"

StateModel::StateModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> StateModel::roleNames() const
{
    return {{TypeRole, "type"}, {StateKeyRole, "stateKey"}};
}
QVariant StateModel::data(const QModelIndex &index, int role) const
{
#ifdef QUOTIENT_07
    auto row = index.row();
    switch (role) {
    case TypeRole:
        return m_stateEvents[row].first;
    case StateKeyRole:
        return m_stateEvents[row].second;
    }
#endif
    return {};
}

int StateModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
#ifdef QUOTIENT_07
    return m_room->currentState().events().size();
#else
    return 0;
#endif
}

NeoChatRoom *StateModel::room() const
{
    return m_room;
}

void StateModel::setRoom(NeoChatRoom *room)
{
    m_room = room;
    Q_EMIT roomChanged();
    beginResetModel();
    m_stateEvents.clear();
#ifdef QUOTIENT_07
    m_stateEvents = m_room->currentState().events().keys();
#endif
    endResetModel();
    connect(room, &NeoChatRoom::changed, this, [this] {
        beginResetModel();
        m_stateEvents.clear();
#ifdef QUOTIENT_07
        m_stateEvents = m_room->currentState().events().keys();
#endif
        endResetModel();
    });
}

QByteArray StateModel::stateEventJson(const QModelIndex &index)
{
    auto row = index.row();
#ifdef QUOTIENT_07
    return QJsonDocument(m_room->currentState().events()[m_stateEvents[row]]->fullJson()).toJson();
#else
    return {};
#endif
}
