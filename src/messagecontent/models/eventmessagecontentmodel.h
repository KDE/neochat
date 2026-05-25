// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "filepreview.h"
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
     * @brief Close the link preview at the given index.
     *
     * If the given index is not a link preview component, nothing happens.
     */
    Q_INVOKABLE void closeLinkPreview(int row);

    /**
     * @brief Reply to the message in a thread.
     *
     * Starts a thread if the message isn't currently threaded. Otherwise a new message
     * will be added to an existing thread.
     */
    Q_INVOKABLE void replyInThread();

    /**
     * @brief Cancel a threaded reply.
     *
     * This removes any chat bar added to allow a threaded reply.
     */
    Q_INVOKABLE void cancelReplyInThread();

    /**
     * @brief Returns the thread model for the given thread root event ID.
     *
     * A model is created if one doesn't exist. Will return nullptr if threadRootId
     * is empty.
     */
    Q_INVOKABLE ThreadModel *modelForThread(const QString &threadRootId);

Q_SIGNALS:
    void eventUpdated();

private:
    void initializeModel();

    NeoChatDateTime dateTime() const override;
    QString authorId() const override;
    QString threadRootId() const override;

    MessageState m_currentState = Unknown;
    bool m_isReply;

    void initializeEvent();
    void getEvent();

    Blocks::Block *unavailableBlock();
    void resetModel();
    void resetContent(bool isEditing = false, bool isThreading = false);
    Blocks::BlockPtrs messageContentComponents(bool isEditing = false, bool isThreading = false);

    Blocks::FilePreviewBlockLoader *m_loader = nullptr;
    bool m_fileChecked = false;
    void checkFilePreview();
    void insertFIlePreview();

    void checkLinkPreview();
    QList<QUrl> m_removedLinkPreviews;
    Blocks::Block *linkPreviewComponent(const QUrl &link);

    void updateReactionModel();
};
