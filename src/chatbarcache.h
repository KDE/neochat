// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "models/messagecontentmodel.h"

namespace Quotient
{
class RoomMember;
}


/**
 * @brief Defines a user mention in the current chat or edit text.
 */
struct Mention {
    QTextCursor cursor; /**< Contains the mention's text and position in the text.  */
    QString text; /**< The inserted text of the mention. */
    int start = 0; /**< Start position of the mention. */
    int position = 0; /**< End position of the mention. */
    QString id; /**< The id the mention (used to create link when sending the message). */
};

/**
 * @class ChatBarCache
 *
 * A class to cache data from a chat bar.
 *
 * A chat bar can be anything that allows users to compose or edit message, it doesn't
 * necessarily have to use the ChatBar component, e.g. ChatBarComponent.
 *
 * This object is intended to allow the current contents of a chat bar to be cached
 * between different rooms, i.e. there is an expectation that each NeoChatRoom could
 * have a separate cache for each chat bar.
 *
 * @note The NeoChatRoom which this component is created in is expected to be set
 *       as it's parent. This is necessary for certain functions which need to get
 *       relevant room information.
 *
 * @sa ChatBar, ChatBarComponent, NeoChatRoom
 */
class ChatBarCache : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The text in the chat bar.
     *
     * Due to problems with QTextDocument, unlike the other properties here,
     * text is *not* used to store the text when switching rooms.
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    /**
     * @brief Whether the chat bar is currently replying to a message.
     */
    Q_PROPERTY(bool isReplying READ isReplying NOTIFY relationIdChanged)

    /**
     * @brief The Matrix message ID of an event being replied to, if any.
     *
     * Will return empty if the RelationType is currently set to None or Edit.
     *
     * @note Replying, editing and attachments are exclusive so setting this will
     *       clear an edit or attachment.
     *
     * @sa RelationType
     */
    Q_PROPERTY(QString replyId READ replyId WRITE setReplyId NOTIFY relationIdChanged)

    /**
     * @brief Whether the chat bar is currently editing a message.
     */
    Q_PROPERTY(bool isEditing READ isEditing NOTIFY relationIdChanged)

    /**
     * @brief The Matrix message ID of an event being edited, if any.
     *
     * Will return empty if the RelationType is currently set to None or Reply.
     *
     * @note Replying, editing and attachments are exclusive so setting this will
     *       clear an reply or attachment.
     *
     * @sa RelationType
     */
    Q_PROPERTY(QString editId READ editId WRITE setEditId NOTIFY relationIdChanged)

    /**
     * @brief Get the RoomMember object for the message being replied to.
     *
     * Returns an empty RoomMember if not replying to a message.
     *
     * @sa Quotient::RoomMember
     */
    Q_PROPERTY(Quotient::RoomMember relationAuthor READ relationAuthor NOTIFY relationIdChanged)

    /**
     * @brief The content of the related message.
     *
     * Will be QString() if no related message.
     */
    Q_PROPERTY(QString relationMessage READ relationMessage NOTIFY relationIdChanged)

    /**
     * @brief Whether the chat bar is replying in a thread.
     */
    Q_PROPERTY(bool isThreaded READ isThreaded NOTIFY threadIdChanged)

    /**
     * @brief The Matrix message ID of thread root event, if any.
     */
    Q_PROPERTY(QString threadId READ threadId WRITE setThreadId NOTIFY threadIdChanged)

    /**
     * @brief The local path for a file to send, if any.
     *
     * @note Replying, editing and attachments are exclusive so setting this will
     *       clear an edit or reply.
     */
    Q_PROPERTY(QString attachmentPath READ attachmentPath WRITE setAttachmentPath NOTIFY attachmentPathChanged)

public:
    /**
     * @brief Describes the type of relation which relationId can refer to.
     *
     * A chat bar can only be relating to a single message at a time making these
     * exclusive.
     */
    enum RelationType {
        Reply, /**< The current relation is a message being replied to. */
        Edit, /**< The current relation is a message being edited. */
        None, /**< There is currently no relation event */
    };
    Q_ENUM(RelationType)

    explicit ChatBarCache(QObject *parent = nullptr);

    QString text() const;
    QString sendText() const;
    void setText(const QString &text);

    bool isReplying() const;
    QString replyId() const;
    void setReplyId(const QString &replyId);

    bool isEditing() const;
    QString editId() const;
    void setEditId(const QString &editId);

    Quotient::RoomMember relationAuthor() const;

    QString relationMessage() const;

    bool isThreaded() const;
    QString threadId() const;
    void setThreadId(const QString &threadId);

    QString attachmentPath() const;
    void setAttachmentPath(const QString &attachmentPath);

    /**
     * @brief Clear all relations in the cache.
     *
     * This includes relation ID, thread root ID and attachment path.
     */
    Q_INVOKABLE void clearRelations();

    /**
     * @brief Retrieve the mentions for the current chat bar text.
     */
    QList<Mention> *mentions();

    /**
     * @brief Get the saved chat bar text.
     */
    QString savedText() const;

    /**
     * @brief Save the chat bar text.
     */
    void setSavedText(const QString &savedText);

    /**
     * @brief Post the contents of the cache as a message in the room.
     */
    Q_INVOKABLE void postMessage();

Q_SIGNALS:
    void textChanged();
    void relationIdChanged(const QString &oldEventId, const QString &newEventId);
    void threadIdChanged(const QString &oldThreadId, const QString &newThreadId);
    void attachmentPathChanged();

private:
    QString m_text = QString();
    QString formatMentions() const;

    QString m_relationId = QString();
    RelationType m_relationType = RelationType::None;
    QString m_threadId = QString();
    QString m_attachmentPath = QString();
    QList<Mention> m_mentions;
    QString m_savedText;

    QPointer<MessageContentModel> m_relationContentModel;

    void clearCache();
};
