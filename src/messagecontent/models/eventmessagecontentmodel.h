// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "models/messagecontentmodel.h"
#include "models/threadmodel.h"

/**
 * @class EventMessageContentModel
 *
 * Inherited from MessageContentModel this visulaises the content of a Quotient::RoomMessageEvent.
 */
class EventMessageContentModel : public MessageContentModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum MessageState {
        Unknown, /**< The message state is unknown. */
        Pending, /**< The message is a new pending message which the server has not yet acknowledged. */
        Available, /**< The message is available and acknowledged by the server. */
        UnAvailable, /**< The message can't be retrieved either because it doesn't exist or is blocked. */
    };
    Q_ENUM(MessageState)

    explicit EventMessageContentModel(NeoChatRoom *room,
                                         const QString &eventId,
                                         bool isReply = false,
                                         bool isPending = false,
                                         MessageContentModel *parent = nullptr);

    /**
     * @brief Returns the thread model for the given thread root event ID.
     *
     * A model is created if one doesn't exist. Will return nullptr if threadRootId
     * is empty.
     */
    Q_INVOKABLE ThreadModel *modelForThread(const QString &threadRootId);

    static void setThreadsEnabled(bool enableThreads);

Q_SIGNALS:
    void eventUpdated();
    void threadsEnabledChanged();

private:
    void initializeModel();

    QDateTime time() const override;
    QString timeString() const override;
    QString authorId() const override;
    QString threadRootId() const override;

    MessageState m_currentState = Unknown;
    bool m_isReply;

    void initializeEvent();
    void getEvent();

    MessageComponent unavailableMessageComponent() const;
    void resetModel();
    void resetContent(bool isEditing = false, bool isThreading = false);
    QList<MessageComponent> messageContentComponents(bool isEditing = false, bool isThreading = false);

    void updateReplyModel();

    QList<MessageComponent> componentsForType(MessageComponentType::Type type);

    void updateItineraryModel();

    void updateReactionModel();

    static bool m_threadsEnabled;
};
