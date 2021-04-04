// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPl-3.0-or-later

#pragma once

#include <QObject>
#include <QVariant>

/// Helper singleton for keeping the chatbar state in sync in the application.
class ChatBoxHelper : public QObject
{
    Q_OBJECT
    /// True, iff the user is currently editing one of their previous message.
    Q_PROPERTY(bool isEditing READ isEditing NOTIFY isEditingChanged)
    Q_PROPERTY(QString editEventId READ editEventId WRITE setEditEventId NOTIFY editEventIdChanged)
    Q_PROPERTY(QString editContent READ editContent WRITE setEditContent NOTIFY editContentChanged)

    Q_PROPERTY(bool isReplying READ isReplying NOTIFY isReplyingChanged)
    Q_PROPERTY(QString replyEventId READ replyEventId WRITE setReplyEventId NOTIFY replyEventIdChanged)
    Q_PROPERTY(QString replyEventContent READ replyEventContent WRITE setReplyEventContent NOTIFY replyEventContentChanged)
    Q_PROPERTY(QVariant replyUser READ replyUser WRITE setReplyUser NOTIFY replyUserChanged)

    Q_PROPERTY(QString attachmentPath READ attachmentPath WRITE setAttachmentPath NOTIFY attachmentPathChanged)
    Q_PROPERTY(bool hasAttachment READ hasAttachment NOTIFY hasAttachmentChanged)

public:
    ChatBoxHelper(QObject *parent = nullptr);
    ~ChatBoxHelper() = default;

    bool isEditing() const;
    QString editEventId() const;
    QString editContent() const;

    QString replyEventId() const;
    QString replyEventContent() const;
    QVariant replyUser() const;
    bool isReplying() const;

    QString attachmentPath() const;
    bool hasAttachment() const;

    void setEditEventId(const QString &editEventId);
    void setEditContent(const QString &editContent);
    void setReplyEventId(const QString &replyEventId);
    void setReplyEventContent(const QString &replyEventContent);
    void setAttachmentPath(const QString &attachmentPath);
    void setReplyUser(const QVariant &replyUser);

    Q_INVOKABLE void replyToMessage(const QString &replyEventid, const QString &replyEvent, const QVariant &replyUser);
    Q_INVOKABLE void edit(const QString &message, const QString &formattedBody, const QString &eventId);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void clearEditReply();
    Q_INVOKABLE void clearAttachment();

Q_SIGNALS:
    void isEditingChanged(bool isEditing);
    void editEventIdChanged(const QString &editEventId);
    void editContentChanged();
    void replyEventIdChanged(const QString &replyEventId);
    void replyEventContentChanged(const QString &replyEventContent);
    void replyUserChanged();
    void isReplyingChanged(bool isReplying);
    void attachmentPathChanged(const QString &attachmentPath);
    void hasAttachmentChanged(bool hasAttachment);
    void editing(const QString &message, const QString &formattedBody);
    void shouldClearText();

private:
    QString m_editEventId;
    QString m_editContent;
    QString m_replyEventId;
    QString m_replyEventContent;
    QVariant m_replyUser;
    QString m_attachmentPath;
};
