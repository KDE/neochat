// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "contentprovider.h"

ContentProvider::ContentProvider()
{
}

ContentProvider &ContentProvider::self()
{
    static ContentProvider instance;
    return instance;
}

MessageContentModel *ContentProvider::contentModelForEvent(NeoChatRoom *room, const QString &evtOrTxnId, bool isReply)
{
    if (!room || evtOrTxnId.isEmpty()) {
        return nullptr;
    }

    if (!m_eventContentModels.contains(evtOrTxnId)) {
        m_eventContentModels.insert(evtOrTxnId, new MessageContentModel(room, evtOrTxnId, isReply));
    }

    return m_eventContentModels.object(evtOrTxnId);
}

MessageContentModel *ContentProvider::contentModelForEvent(NeoChatRoom *room, const Quotient::RoomEvent *event, bool isReply)
{
    if (!room) {
        return nullptr;
    }
    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event);
    if (roomMessageEvent == nullptr) {
        // If for some reason a model is there remove.
        if (m_eventContentModels.contains(event->id())) {
            m_eventContentModels.remove(event->id());
        }
        if (m_eventContentModels.contains(event->transactionId())) {
            m_eventContentModels.remove(event->transactionId());
        }
        return nullptr;
    }

    if (event->isStateEvent() || event->matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
        return nullptr;
    }

    auto eventId = event->id();
    const auto txnId = event->transactionId();
    if (!m_eventContentModels.contains(eventId) && !m_eventContentModels.contains(txnId)) {
        m_eventContentModels.insert(eventId.isEmpty() ? txnId : eventId,
                                    new MessageContentModel(room, eventId.isEmpty() ? txnId : eventId, isReply, eventId.isEmpty()));
    }

    if (!eventId.isEmpty() && m_eventContentModels.contains(eventId)) {
        return m_eventContentModels.object(eventId);
    }

    if (!txnId.isEmpty() && m_eventContentModels.contains(txnId)) {
        if (eventId.isEmpty()) {
            return m_eventContentModels.object(txnId);
        }

        // If we now have an event ID use that as the map key instead of transaction ID.
        auto txnModel = m_eventContentModels.take(txnId);
        m_eventContentModels.insert(eventId, txnModel);
        return m_eventContentModels.object(eventId);
    }

    return nullptr;
}

ThreadModel *ContentProvider::modelForThread(NeoChatRoom *room, const QString &threadRootId)
{
    if (!room || threadRootId.isEmpty()) {
        return nullptr;
    }

    if (!m_threadModels.contains(threadRootId)) {
        m_threadModels.insert(threadRootId, new ThreadModel(threadRootId, room));
    }

    return m_threadModels.object(threadRootId);
}
