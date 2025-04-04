// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QCache>

#include "models/messagecontentmodel.h"
#include "models/threadmodel.h"
#include "neochatroom.h"

/**
 * @class ContentProvider
 *
 * Store and retrieve models for message content.
 */
class ContentProvider
{
public:
    /**
     * Get the global instance of ContentProvider.
     */
    static ContentProvider &self();

    /**
     * @brief Returns the content model for the given event ID.
     *
     * A model is created if one doesn't exist. Will return nullptr if evtOrTxnId
     * is empty.
     *
     * @warning If a non-empty ID is given it is assumed to be a valid Quotient::RoomMessageEvent
     *          event ID. The caller must ensure that the ID is a real event. A model will be
     *          returned unconditionally.
     *
     * @warning Do NOT use for pending events as this function has no way to differentiate.
     */
    MessageContentModel *contentModelForEvent(NeoChatRoom *room, const QString &evtOrTxnId, bool isReply = false);

    /**
     * @brief Returns the content model for the given event.
     *
     * A model is created if one doesn't exist. Will return nullptr if event is:
     *  - nullptr
     *  - not a Quotient::RoomMessageEvent (e.g a state event)
     *
     * @note This method is preferred to the version using just an event ID as it
     *       can perform some basic checks. If a copy of the event is not available,
     *       you may have to use the version that takes an event ID.
     *
     * @note This version must be used for pending events as it can differentiate.
     */
    MessageContentModel *contentModelForEvent(NeoChatRoom *room, const Quotient::RoomEvent *event, bool isReply = false);

    /**
     * @brief Returns the thread model for the given thread root event ID.
     *
     * A model is created if one doesn't exist. Will return nullptr if threadRootId
     * is empty.
     */
    ThreadModel *modelForThread(NeoChatRoom *room, const QString &threadRootId);

private:
    ContentProvider();

    QCache<QString, MessageContentModel> m_eventContentModels;
    QCache<QString, ThreadModel> m_threadModels;
};
