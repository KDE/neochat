// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "roomlastmessageprovider.h"

using namespace Qt::Literals::StringLiterals;

RoomLastMessageProvider::RoomLastMessageProvider()
    : m_config(KSharedConfig::openStateConfig())
    , m_configGroup(KConfigGroup(m_config, u"EventCache"_s))
{
}

RoomLastMessageProvider::~RoomLastMessageProvider()
{
    m_config->sync();
}

RoomLastMessageProvider &RoomLastMessageProvider::self()
{
    static RoomLastMessageProvider instance;
    return instance;
}

bool RoomLastMessageProvider::hasKey(const QString &roomId) const
{
    return m_configGroup.hasKey(roomId);
}

QByteArray RoomLastMessageProvider::read(const QString &roomId) const
{
    return m_configGroup.readEntry(roomId, QByteArray{});
}

void RoomLastMessageProvider::write(const QString &roomId, const QByteArray &json)
{
    m_configGroup.writeEntry(roomId, json);
}
