// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <Quotient/events/roommessageevent.h>

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

    /**
     * @brief The room that messages will be sent to.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    explicit ActionsHandler(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

Q_SIGNALS:
    void roomChanged();
    void showEffect(const QString &effect);

public Q_SLOTS:

    /**
     * @brief Pre-process text and send message.
     */
    void handleNewMessage();

    /**
     * @brief Pre-process text and send edit.
     */
    void handleEdit();

private:
    NeoChatRoom *m_room = nullptr;
    void checkEffects(const QString &text);

    QString handleMentions(QString handledText, const bool &isEdit = false);
    void handleMessage(const QString &text, QString handledText, const bool &isEdit = false);
};
