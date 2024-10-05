// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class ChatBarCache;
class NeoChatRoom;

/**
 * @class ActionsHandler
 *
 * This class contains functions to handle chat messages ready for posting to a room.
 *
 * Everything that needs to be done to prepare the message for posting in a room
 * including:
 *  - File handling
 *  - User mentions
 *  - Quick edits
 *  - Chat actions
 *  - Custom emojis
 *
 * @note A chat action is a message starting with /, resulting in something other
 *       than a normal message being sent (e.g. /me, /join).
 *
 * @sa ActionsModel, NeoChatRoom
 */
class ActionsHandler
{
public:
    /**
     * @brief Pre-process text and send message event.
     */
    static void handleMessageEvent(NeoChatRoom *room, ChatBarCache *chatBarCache);

private:
    static QString handleMentions(ChatBarCache *chatBarCache);
    static bool handleQuickEdit(NeoChatRoom *room, const QString &handledText);

    static void handleMessage(NeoChatRoom *room, QString handledText, ChatBarCache *chatBarCache);
};
