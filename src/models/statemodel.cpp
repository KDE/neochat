// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "statemodel.h"

StateModel::StateModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> StateModel::roleNames() const
{
    return {{TypeRole, "type"}, {StateKeyRole, "stateKey"}, {SourceRole, "source"}};
}
QVariant StateModel::data(const QModelIndex &index, int role) const
{
#ifdef QUOTIENT_07
    auto row = index.row();
    switch (role) {
    case TypeRole:
        return m_room->currentState().events().keys()[row].first;
    case StateKeyRole:
        return m_room->currentState().events().keys()[row].second;
    case SourceRole:
        return QJsonDocument(m_room->currentState().events()[m_room->currentState().events().keys()[row]]->fullJson()).toJson();
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
    endResetModel();
    connect(room, &NeoChatRoom::changed, this, [=] {
        beginResetModel();
        endResetModel();
    });
}
