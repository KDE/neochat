// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <KConfigGroup>
#include <KSharedConfig>

/**
 * Store and retrieve the last message of a room.
 */
class RoomLastMessageProvider
{
public:
    /**
     * Get the global instance of RoomLastMessageProvider.
     */
    static RoomLastMessageProvider &self();
    ~RoomLastMessageProvider();

    /**
     * Check if we have the last message content for the specified roomId.
     */
    bool hasKey(const QString &roomId) const;

    /**
     * Read the last message content of the specified roomId.
     */
    QByteArray read(const QString &roomId) const;

    /**
     * Write the last message content for the specified roomId.
     */
    void write(const QString &roomId, const QByteArray &json);

private:
    RoomLastMessageProvider();

    KSharedConfig::Ptr m_config;
    KConfigGroup m_configGroup;
};
