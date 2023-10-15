// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarcache.h"

#include "eventhandler.h"
#include "neochatroom.h"

ChatBarCache::ChatBarCache(QObject *parent)
    : QObject(parent)
{
}

QString ChatBarCache::text() const
{
    return m_text;
}

void ChatBarCache::setText(const QString &text)
{
    if (text == m_text) {
        return;
    }
    m_text = text;
    Q_EMIT textChanged();
}

bool ChatBarCache::isReplying() const
{
    return m_relationType == Reply && !m_relationId.isEmpty();
}

QString ChatBarCache::replyId() const
{
    if (m_relationType != Reply) {
        return {};
    }
    return m_relationId;
}

void ChatBarCache::setReplyId(const QString &replyId)
{
    if (m_relationType == Reply && m_relationId == replyId) {
        return;
    }
    m_relationId = replyId;
    if (m_relationId.isEmpty()) {
        m_relationType = None;
    } else {
        m_relationType = Reply;
    }
    m_attachmentPath = QString();
    Q_EMIT relationIdChanged();
}

bool ChatBarCache::isEditing() const
{
    return m_relationType == Edit && !m_relationId.isEmpty();
}

QString ChatBarCache::editId() const
{
    if (m_relationType != Edit) {
        return {};
    }
    return m_relationId;
}

void ChatBarCache::setEditId(const QString &editId)
{
    if (m_relationType == Edit && m_relationId == editId) {
        return;
    }
    m_relationId = editId;
    if (m_relationId.isEmpty()) {
        m_relationType = None;
    } else {
        m_relationType = Edit;
    }
    m_attachmentPath = QString();
    Q_EMIT relationIdChanged();
}

QVariantMap ChatBarCache::relationUser() const
{
    if (parent() == nullptr) {
        qWarning() << "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.";
        return {};
    }
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.";
        return {};
    }
    if (m_relationId.isEmpty()) {
        return room->getUser(nullptr);
    }
    return room->getUser(room->user((*room->findInTimeline(m_relationId))->senderId()));
}

QString ChatBarCache::relationMessage() const
{
    if (parent() == nullptr) {
        qWarning() << "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.";
        return {};
    }
    if (m_relationId.isEmpty()) {
        return {};
    }
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.";
        return {};
    }
    EventHandler eventhandler;
    eventhandler.setRoom(room);
    if (auto event = room->findInTimeline(m_relationId); event != room->historyEdge()) {
        eventhandler.setEvent(&**event);
        return eventhandler.getPlainBody();
    }
    return {};
}

QString ChatBarCache::attachmentPath() const
{
    return m_attachmentPath;
}

void ChatBarCache::setAttachmentPath(const QString &attachmentPath)
{
    if (attachmentPath == m_attachmentPath) {
        return;
    }
    m_attachmentPath = attachmentPath;
    m_relationType = None;
    m_relationId = QString();
    Q_EMIT attachmentPathChanged();
}

QVector<Mention> *ChatBarCache::mentions()
{
    return &m_mentions;
}

QString ChatBarCache::savedText() const
{
    return m_savedText;
}

void ChatBarCache::setSavedText(const QString &savedText)
{
    m_savedText = savedText;
}
