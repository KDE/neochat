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
        QUrl url(dynamic_cast<const Blocks::UrlCacheItem *>(m_cache.at(0))->source);
        auto path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        return path.mid(path.lastIndexOf(u'/') + 1);
    }

    return cacheText;
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
    QString id;
    if (!m_relationId.isEmpty()) {
        id = m_relationId;
    }
    if (!m_cache.empty() && m_cache.at(0)->type == Blocks::Reply) {
        if (const auto replyCacheItem = dynamic_cast<const Blocks::ReplyCacheItem *>(m_cache.at(0))) {
            id = replyCacheItem->id;
        }
    }
    if (id.isEmpty()) {
        return m_room->member({});
    }
    const auto [event, _] = m_room->getEvent(id);
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
    QString id;
    if (!m_relationId.isEmpty()) {
        id = m_relationId;
    }
    if (!m_cache.empty() && m_cache.at(0)->type == Blocks::Reply) {
        if (const auto replyCacheItem = dynamic_cast<const Blocks::ReplyCacheItem *>(m_cache.at(0))) {
            id = replyCacheItem->id;
        }
    }
    if (id.isEmpty()) {
        return {};
    }
    if (auto [event, _] = m_room->getEvent(id); event != nullptr) {
        return EventHandler::rawMessageBody(*event);
    }
    return {};
}

Blocks::BlockPtrs ChatBarCache::relationComponents(QObject *parent) const
{
    if (!m_room) {
        qCWarning(ChatBar) << "ChatBarCache:" << __FUNCTION__ << "called after room was deleted";
        return {};
    }
    QString id;
    if (!m_relationId.isEmpty()) {
        id = m_relationId;
    }
    if (!m_cache.empty() && m_cache.at(0)->type == Blocks::Reply) {
        if (const auto replyCacheItem = dynamic_cast<const Blocks::ReplyCacheItem *>(m_cache.at(0))) {
            id = replyCacheItem->id;
        }
    }
    if (id.isEmpty()) {
        return {};
    }
    if (auto [event, _] = m_room->getEvent(id); event != nullptr) {
        return TextHandler()
            .textComponents(EventHandler::rawMessageBody(*event), EventHandler::messageBodyInputFormat(*event), m_room, event, false, false, parent);
    }
    return {};
}

void ChatBarCache::clearRelations()
{
    const auto oldEventId = std::exchange(m_relationId, QString());
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
}

void ChatBarCache::postMessage(const QString &threadRootId)
{
    if (!m_room) {
        qCWarning(ChatBar) << "ChatBarCache:" << __FUNCTION__ << "called after room was deleted";
        return;
    }

    bool isReply = m_cache.at(0)->type == Blocks::Reply;
    QString replyId;
    if (const auto replyCacheItem = dynamic_cast<const Blocks::ReplyCacheItem *>(m_cache.at(0))) {
        replyId = replyCacheItem->id;
    }
    std::optional<Quotient::EventRelation> relatesTo = std::nullopt;

    if (!threadRootId.isEmpty()) {
        relatesTo = Quotient::EventRelation::replyInThread(threadRootId, !isReply, isReply ? replyId : threadRootId);
    } else if (!editId().isEmpty()) {
        relatesTo = Quotient::EventRelation::replace(editId());
    } else if (isReply) {
        relatesTo = Quotient::EventRelation::replyTo(replyId);
    }

    if (Blocks::isFileType(m_cache.at(0)->type)) {
        const auto fileCacheItem = dynamic_cast<const Blocks::UrlCacheItem *>(m_cache.at(0));
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

    auto body = TextHandler::stripMatrixLinks(m_cache.toString());
    body = TextHandler::unescapeBackslashes(body);
    // We want to strip Matrix links here because it matches Element behavior, but more importantly is less annoying in bridged chats.
    m_room->post<Quotient::RoomMessageEvent>(body, *std::get<std::optional<Quotient::RoomMessageEvent::MsgType>>(result), std::move(content), relatesTo);
    clearCache();
}

void ChatBarCache::clearCache()
{
    m_cache.clear();
    clearRelations();
}

#include "moc_chatbarcache.cpp"
