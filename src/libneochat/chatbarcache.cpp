// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarcache.h"

#include <QMimeData>

#include <KUrlMimeData>

#include <Quotient/roommember.h>

#include "block.h"
#include "blockcache.h"
#include "blocktype.h"
#include "eventhandler.h"
#include "models/actionsmodel.h"
#include "neochatroom.h"
#include "texthandler.h"

#include "chatbarlogging.h"

using namespace Qt::StringLiterals;

ChatBarCache::ChatBarCache(NeoChatRoom *room)
    : QObject(room)
    , m_room(room)
{
    Q_ASSERT(room);
    connect(room, &NeoChatRoom::memberLeft, this, &ChatBarCache::relationAuthorIsPresentChanged);
    connect(room, &NeoChatRoom::memberJoined, this, &ChatBarCache::relationAuthorIsPresentChanged);
    connect(this, &ChatBarCache::relationIdChanged, this, &ChatBarCache::relationAuthorIsPresentChanged);
}

Blocks::Cache &ChatBarCache::cache()
{
    return m_cache;
}

QString ChatBarCache::sendText() const
{
    const auto cacheText = m_cache.toString();
    if (cacheText.isEmpty() && Blocks::isFileType(m_cache.at(0)->type)) {
        QUrl url(dynamic_cast<const Blocks::FileCacheItem *>(m_cache.at(0))->source);
        auto path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        return path.mid(path.lastIndexOf(u'/') + 1);
    }

    return cacheText;
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
    const auto oldEventId = std::exchange(m_relationId, replyId);
    if (m_relationId.isEmpty()) {
        m_relationType = None;
    } else {
        m_relationType = Reply;
    }
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
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
    const auto oldEventId = std::exchange(m_relationId, editId);
    if (m_relationId.isEmpty()) {
        m_relationType = None;
    } else {
        m_relationType = Edit;
    }
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
}

Quotient::RoomMember ChatBarCache::relationAuthor() const
{
    if (!m_room) {
        qCWarning(ChatBar) << "ChatBarCache:" << __FUNCTION__ << "called after room was deleted";
        return {};
    }
    if (m_relationId.isEmpty()) {
        return m_room->member({});
    }
    const auto [event, _] = m_room->getEvent(m_relationId);
    if (event != nullptr) {
        return m_room->member(event->senderId());
    }
    qCWarning(ChatBar) << "Failed to find relation" << m_relationId << "in timeline?";
    return m_room->member(QString());
}

bool ChatBarCache::relationAuthorIsPresent() const
{
    return relationAuthor().membershipState() == Quotient::Membership::Join;
}

QString ChatBarCache::relationMessage() const
{
    if (!m_room) {
        qCWarning(ChatBar) << "ChatBarCache:" << __FUNCTION__ << "called after room was deleted";
        return {};
    }
    if (m_relationId.isEmpty()) {
        return {};
    }
    if (auto [event, _] = m_room->getEvent(m_relationId); event != nullptr) {
        return EventHandler::rawMessageBody(*event);
    }
    return {};
}

QList<Blocks::Block> ChatBarCache::relationComponents() const
{
    if (!m_room) {
        qCWarning(ChatBar) << "ChatBarCache:" << __FUNCTION__ << "called after room was deleted";
        return {};
    }
    if (m_relationId.isEmpty()) {
        return {};
    }
    if (auto [event, _] = m_room->getEvent(m_relationId); event != nullptr) {
        return TextHandler().textComponents(EventHandler::rawMessageBody(*event), EventHandler::messageBodyInputFormat(*event), m_room, event);
    }
    return {};
}

bool ChatBarCache::isThreaded() const
{
    return !m_threadId.isEmpty();
}

QString ChatBarCache::threadId() const
{
    return m_threadId;
}

void ChatBarCache::setThreadId(const QString &threadId)
{
    if (m_threadId == threadId) {
        return;
    }
    const auto oldThreadId = std::exchange(m_threadId, threadId);
    Q_EMIT threadIdChanged(oldThreadId, m_threadId);
}

void ChatBarCache::clearRelations()
{
    const auto oldEventId = std::exchange(m_relationId, QString());
    const auto oldThreadId = std::exchange(m_threadId, QString());
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
    Q_EMIT threadIdChanged(oldThreadId, m_threadId);
}

QString ChatBarCache::savedText() const
{
    return m_savedText;
}

void ChatBarCache::setSavedText(const QString &savedText)
{
    m_savedText = savedText;
}

void ChatBarCache::postMessage()
{
    if (!m_room) {
        qCWarning(ChatBar) << "ChatBarCache:" << __FUNCTION__ << "called after room was deleted";
        return;
    }

    bool isReply = !replyId().isEmpty();
    std::optional<Quotient::EventRelation> relatesTo = std::nullopt;

    if (!threadId().isEmpty()) {
        relatesTo = Quotient::EventRelation::replyInThread(threadId(), !isReply, isReply ? replyId() : threadId());
    } else if (!editId().isEmpty()) {
        relatesTo = Quotient::EventRelation::replace(editId());
    } else if (isReply) {
        relatesTo = Quotient::EventRelation::replyTo(replyId());
    }

    if (Blocks::isFileType(m_cache.at(0)->type)) {
        const auto fileCacheItem = dynamic_cast<const Blocks::FileCacheItem *>(m_cache.at(0));
        m_room->uploadFile(fileCacheItem->source, sendText(), relatesTo);
        clearCache();
        return;
    }

    const auto result = ActionsModel::handleAction(m_room, this);
    if (!result.second.has_value()) {
        clearCache();
        return;
    }

    TextHandler textHandler;
    textHandler.setData(*std::get<std::optional<QString>>(result));
    const auto sendText = textHandler.handleSendText();

    if (sendText.length() == 0) {
        return;
    }

    auto content = std::make_unique<Quotient::EventContent::TextContent>(sendText, u"text/html"_s);

    m_room->post<Quotient::RoomMessageEvent>(m_cache.toString(),
                                           *std::get<std::optional<Quotient::RoomMessageEvent::MsgType>>(result),
                                           std::move(content),
                                           relatesTo);
    clearCache();
}

void ChatBarCache::clearCache()
{
    m_cache.clear();
    m_savedText = QString();
    clearRelations();
}

#include "moc_chatbarcache.cpp"
