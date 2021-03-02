// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "callparticipantsmodel.h"

QVariant CallParticipantsModel::data(const QModelIndex &index, int role) const
{
    if (role == ObjectRole) {
        return QVariant::fromValue(m_callParticipants[index.row()]);
    }
    return {};
}

int CallParticipantsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_callParticipants.size();
}

void CallParticipantsModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_callParticipants.size() - 1);
    m_callParticipants.clear();
    endRemoveRows();
}

CallParticipant *CallParticipantsModel::callParticipantForUser(NeoChatUser *user)
{
    for (const auto &callParticipant : m_callParticipants) {
        if (callParticipant->m_user == user) {
            return callParticipant;
        }
    }
    return nullptr;
}

QHash<int, QByteArray> CallParticipantsModel::roleNames() const
{
    return {
        {WidthRole, "width"},
        {HeightRole, "height"},
        {PadRole, "pad"},
        {ObjectRole, "object"},
    };
}

void CallParticipantsModel::addParticipant(CallParticipant *callParticipant)
{
    beginInsertRows(QModelIndex(), m_callParticipants.size(), m_callParticipants.size());
    m_callParticipants += callParticipant;
    endInsertRows();
}

void CallParticipantsModel::setHasCamera(NeoChatUser *user, bool hasCamera)
{
    callParticipantForUser(user)->m_hasCamera = hasCamera;
    Q_EMIT callParticipantForUser(user)->hasCameraChanged();
}
