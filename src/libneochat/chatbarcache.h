// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QTextCursor>

#include "blockcache.h"

namespace Quotient
{
class RoomMember;
}

struct MessageComponent;
class NeoChatRoom;

/**
 * @class ChatBarCache
 *
 * A class to cache data from a chat bar.
 *
 * A chat bar can be anything that allows users to compose or edit message, it doesn't
 * necessarily have to use the ChatBar component.
 *
 * This object is intended to allow the current contents of a chat bar to be cached
 * between different rooms, i.e. there is an expectation that each NeoChatRoom could
 * have a separate cache for each chat bar.
 *
 * @note The NeoChatRoom which this component is created in is expected to be set
 *       as it's parent. This is necessary for certain functions which need to get
 *       relevant room information.
 *
 * @sa ChatBar, NeoChatRoom
 */
class ChatBarCache : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

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
     * @brief If the author for the message being replied to is still present in the room.
     *
     * @sa Quotient::RoomMember
     */
    Q_PROPERTY(bool relationAuthorIsPresent READ relationAuthorIsPresent NOTIFY relationAuthorIsPresentChanged)

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

    explicit ChatBarCache(NeoChatRoom *room);

    Block::Cache &cache();
    QString sendText() const;

    bool isReplying() const;
    QString replyId() const;
    void setReplyId(const QString &replyId);

    bool isEditing() const;
    QString editId() const;
    void setEditId(const QString &editId);

    Quotient::RoomMember relationAuthor() const;
    bool relationAuthorIsPresent() const;

    QString relationMessage() const;
    QList<MessageComponent> relationComponents() const;

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

    Q_INVOKABLE void drop(QList<QUrl> urls, const QString &transferPortal);

Q_SIGNALS:
    void relationIdChanged(const QString &oldEventId, const QString &newEventId);
    void threadIdChanged(const QString &oldThreadId, const QString &newThreadId);
    void attachmentPathChanged();
    void mentionAdded(const QString &text, const QString &hRef);
    void relationAuthorIsPresentChanged();

private:
    Block::Cache m_cache;
    QPointer<NeoChatRoom> m_room;

    QString m_relationId = QString();
    RelationType m_relationType = RelationType::None;
    QString m_threadId = QString();
    QString m_attachmentPath = QString();
    QString m_savedText;

    void clearCache();
};
