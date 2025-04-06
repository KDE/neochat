// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QCache>
#include <QObject>
#include <QQmlEngine>

#include "models/messagecontentmodel.h"
#include "models/threadmodel.h"
#include "neochatroom.h"
#include "pollhandler.h"

/**
 * @class ContentProvider
 *
 * Store and retrieve models for message content.
 */
class ContentProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /**
     * Get the global instance of ContentProvider.
     */
    static ContentProvider &self();
    static ContentProvider *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&self(), QQmlEngine::CppOwnership);
        return &self();
    }

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
    Q_INVOKABLE MessageContentModel *contentModelForEvent(NeoChatRoom *room, const QString &evtOrTxnId, bool isReply = false);

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

    /**
     * @brief Get a PollHandler object for the given event Id.
     *
     * Will return an existing PollHandler if one already exists for the event ID.
     * A new PollHandler will be created if one doesn't exist.
     *
     * @sa PollHandler
     */
    Q_INVOKABLE PollHandler *handlerForPoll(NeoChatRoom *room, const QString &eventId);

private:
    explicit ContentProvider(QObject *parent = nullptr);

    QCache<QString, MessageContentModel> m_eventContentModels;
    QCache<QString, ThreadModel> m_threadModels;
    QCache<QString, PollHandler> m_pollHandlers;
};
