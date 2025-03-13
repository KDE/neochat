// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "livekitlogmodel.h"

using namespace livekit::proto;

QVariant LivekitLogModel::data(const QModelIndex &index, int role) const
{
    const auto &message = m_messages[index.row()];
    if (role == MessageRole) {
        return message.message();
    }
    return {};
}

int LivekitLogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QHash<int, QByteArray> LivekitLogModel::roleNames() const
{
    return {
        {MessageRole, "message"},
    };
}

void LivekitLogModel::addMessages(livekit::proto::LogRecordRepeated messages)
{
    for (const auto &message : messages) {
        // if (message.level() < 3) {
        beginInsertRows({}, m_messages.size(), m_messages.size() + 1);
        m_messages += message;
        endInsertRows();
        // }
    }
}
