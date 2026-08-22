// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include "filepreview.h"
#include "models/messagecontentmodel.h"
#include "models/threadmodel.h"
#include "neochatroom.h"

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

    /**
     * @brief The room the event was posted to.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The matrix ID for the event.
     */
    Q_PROPERTY(QString eventId READ eventId CONSTANT)

    /**
     * @brief The author of the event.
     */
    Q_PROPERTY(NeochatRoomMember *author READ author NOTIFY eventUpdated)

    /**
     * @brief The date and time the event was sent.
     */
    Q_PROPERTY(NeoChatDateTime dateTime READ dateTime NOTIFY eventUpdated)

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
                                      QObject *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QString eventId() const;
    NeochatRoomMember *author() const;
    NeoChatDateTime dateTime() const;

    /**
     * @brief Close the link preview at the given index.
     *
     * If the given index is not a link preview component, nothing happens.
     */
    Q_INVOKABLE void closeLinkPreview(int row);

    void editEvent();
    Q_INVOKABLE void cancelEventEdit();

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

    Q_INVOKABLE void cancelChatBar();

    /**
     * @brief Returns the thread model for the given thread root event ID.
     *
     * A model is created if one doesn't exist. Will return nullptr if threadRootId
     * is empty.
     */
    Q_INVOKABLE ThreadModel *modelForThread(const QString &threadRootId);

    static bool richTextActive;

    /**
     * @brief Hides the media contained in this message.
     */
    Q_INVOKABLE void hideMedia();

    /**
     * @brief Shows the media contained in this message.
     */
    Q_INVOKABLE void showMedia();

    /**
     * @brief If the media is hidden for a given event.
     */
    bool isMediaHidden();

    static void setSetMediaHidden(std::function<void(const QString &, bool)> func);
    static void setMediaShouldBeHidden(std::function<bool(const QString &)> func);

Q_SIGNALS:
    void roomChanged(NeoChatRoom *oldRoom, NeoChatRoom *newRoom);
    void eventUpdated();

private:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;

    void initializeModel();

    QString authorId() const;
    QString threadRootId() const override;

    MessageState m_currentState = Unknown;
    bool m_isReply;
    bool m_isEditing = false;
    void fillEditCache();

    void initializeEvent();
    void getEvent();

    Blocks::Block *unavailableBlock();
    void resetModel();
    void resetContent(bool isThreading = false);
    Blocks::BlockPtrs messageContentComponents(bool isThreading = false);

    Blocks::FilePreviewBlockLoader *m_loader = nullptr;
    bool m_fileChecked = false;
    void checkFilePreview();
    void insertFilePreview();

    void checkLinkPreview();
    QList<QUrl> m_removedLinkPreviews;
    Blocks::Block *linkPreviewComponent(const QUrl &link);

    void updateReactionModel();

    void setMediaHidden(bool mediaHidden);

    static std::function<void(const QString &, bool)> m_setMediaHidden;
    static std::function<bool(const QString &)> m_mediaShouldBeHidden;
};

class ReplyModelHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    ReplyModelHelper(QObject *parent = nullptr);

    Q_INVOKABLE EventMessageContentModel *modelForEvent(NeoChatRoom *room, const QString &eventId);
};
