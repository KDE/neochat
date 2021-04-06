// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chatboxhelper.h"
#include <QDebug>

ChatBoxHelper::ChatBoxHelper(QObject *parent)
    : QObject(parent)
{
}

bool ChatBoxHelper::isEditing() const
{
    return !m_editEventId.isEmpty();
}

QString ChatBoxHelper::editEventId() const
{
    return m_editEventId;
}

void ChatBoxHelper::setEditEventId(const QString &editEventId)
{
    if (m_editEventId == editEventId) {
        return;
    }

    m_editEventId = editEventId;
    Q_EMIT editEventIdChanged(m_editEventId);
    Q_EMIT isEditingChanged(!m_editEventId.isEmpty());
}

QString ChatBoxHelper::editContent() const
{
    return m_editContent;
}

void ChatBoxHelper::setEditContent(const QString &editContent)
{
    if (m_editContent == editContent) {
        return;
    }

    m_editContent = editContent;
    Q_EMIT editContentChanged();
}

QString ChatBoxHelper::replyEventId() const
{
    return m_replyEventId;
}

void ChatBoxHelper::setReplyEventId(const QString &replyEventId)
{
    if (m_replyEventId == replyEventId) {
        return;
    }

    m_replyEventId = replyEventId;
    Q_EMIT replyEventIdChanged(m_replyEventId);
}

QString ChatBoxHelper::replyEventContent() const
{
    return m_replyEventContent;
}

void ChatBoxHelper::setReplyEventContent(const QString &replyEventContent)
{
    if (m_replyEventContent == replyEventContent) {
        return;
    }

    m_replyEventContent = replyEventContent;
    Q_EMIT replyEventContentChanged(m_replyEventContent);
    Q_EMIT isReplyingChanged(!m_replyEventContent.isEmpty());
}

bool ChatBoxHelper::isReplying() const
{
    return !m_replyEventId.isEmpty();
}

QString ChatBoxHelper::attachmentPath() const
{
    return m_attachmentPath;
}

void ChatBoxHelper::setAttachmentPath(const QString &attachmentPath)
{
    if (m_attachmentPath == attachmentPath) {
        return;
    }

    m_attachmentPath = attachmentPath;
    Q_EMIT attachmentPathChanged(m_attachmentPath);
    Q_EMIT hasAttachmentChanged(!m_attachmentPath.isEmpty());
}

bool ChatBoxHelper::hasAttachment() const
{
    return !m_attachmentPath.isEmpty();
}

void ChatBoxHelper::replyToMessage(const QString &replyEventId, const QString &replyEvent, const QVariant &replyUser)
{
    setEditEventId(QString());
    setEditContent(QString());
    setReplyEventId(replyEventId);
    setReplyEventContent(replyEvent);
    setReplyUser(replyUser);
}

QVariant ChatBoxHelper::replyUser() const
{
    return m_replyUser;
}

void ChatBoxHelper::setReplyUser(const QVariant &replyUser)
{
    if (m_replyUser == replyUser) {
        return;
    }
    m_replyUser = replyUser;
    Q_EMIT replyUserChanged();
}

void ChatBoxHelper::clear()
{
    setEditEventId(QString());
    setEditContent(QString());
    setReplyEventId(QString());
    setReplyEventContent(QString());
    setAttachmentPath(QString());
    setReplyUser(QVariant());
}

void ChatBoxHelper::edit(const QString &message, const QString &formattedBody, const QString &eventId)
{
    setEditEventId(eventId);
    setEditContent(message);
    Q_EMIT editing(message, formattedBody);
}

void ChatBoxHelper::clearEditReply()
{
    setEditEventId(QString());
    setEditContent(QString());
    setReplyEventId(QString());
    setReplyEventContent(QString());
    setReplyUser(QVariant());
    Q_EMIT shouldClearText();
}

void ChatBoxHelper::clearAttachment()
{
    setAttachmentPath(QString());
}
