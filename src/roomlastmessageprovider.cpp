// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "roomlastmessageprovider.h"

using namespace Qt::Literals::StringLiterals;

RoomLastMessageState::RoomLastMessageState()
    : m_config(KSharedConfig::openConfig(u"data"_s, KConfig::SimpleConfig, QStandardPaths::AppDataLocation))
    , m_configGroup(KConfigGroup(m_config, u"EventCache"_s))
{
}

RoomLastMessageState::~RoomLastMessageState()
{
    m_config->sync();
}

RoomLastMessageState &RoomLastMessageState::self()
{
    static RoomLastMessageState instance;
    return instance;
}

bool RoomLastMessageState::hasKey(const QString &roomId) const
{
    return m_configGroup.hasKey(roomId);
}

QByteArray RoomLastMessageState::read(const QString &roomId) const
{
    return m_configGroup.readEntry(roomId, QByteArray{});
}

void RoomLastMessageState::write(const QString &roomId, const QByteArray &json)
{
    m_configGroup.writeEntry(roomId, json);
}
