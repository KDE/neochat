// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <Quotient/events/roommessageevent.h>

#include "chatbarcache.h"
#include "neochatroom.h"

class NeoChatRoom;

/**
 * @class ActionsHandler
 *
 * This class handles chat messages ready for posting to a room.
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
class ActionsHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    explicit ActionsHandler(QObject *parent = nullptr);

    /**
     * @brief The room that messages will be sent to.
     */
    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

Q_SIGNALS:
    void showEffect(const QString &effect);

public Q_SLOTS:
    /**
     * @brief Pre-process text and send message event.
     */
    void handleMessageEvent(ChatBarCache *chatBarCache);

private:
    NeoChatRoom *m_room = nullptr;
    void checkEffects(const QString &text);

    QString handleMentions(QString handledText, QList<Mention> *mentions);
    void handleMessage(const QString &text, QString handledText, ChatBarCache *chatBarCache);
};
