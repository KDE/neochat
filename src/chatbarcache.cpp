// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarcache.h"

#include <Quotient/roommember.h>

#include "chatdocumenthandler.h"
#include "eventhandler.h"
#include "models/actionsmodel.h"
#include "neochatroom.h"
#include "texthandler.h"

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

QString ChatBarCache::sendText() const
{
    if (!attachmentPath().isEmpty()) {
        QUrl url(attachmentPath());
        auto path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        return text().isEmpty() ? path.mid(path.lastIndexOf(u'/') + 1) : text();
    }

    return formatMentions();
}

QString ChatBarCache::formatMentions() const
{
    auto mentions = m_mentions;
    std::sort(mentions.begin(), mentions.end(), [](const auto &a, const auto &b) {
        return a.cursor.anchor() > b.cursor.anchor();
    });

    auto formattedText = text();
    for (const auto &mention : mentions) {
        if (mention.text.isEmpty() || mention.id.isEmpty()) {
            continue;
        }
        formattedText = formattedText.replace(mention.cursor.anchor(),
                                              mention.cursor.position() - mention.cursor.anchor(),
                                              QStringLiteral("[%1](https://matrix.to/#/%2)").arg(mention.text.toHtmlEscaped(), mention.id));
    }

    return formattedText;
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
    m_attachmentPath = QString();
    delete m_relationContentModel;
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
    Q_EMIT attachmentPathChanged();
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
    m_attachmentPath = QString();
    delete m_relationContentModel;
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
    Q_EMIT attachmentPathChanged();
}

Quotient::RoomMember ChatBarCache::relationAuthor() const
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
        return room->member(QString());
    }
    return room->member((*room->findInTimeline(m_relationId))->senderId());
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

    if (auto event = room->findInTimeline(m_relationId); event != room->historyEdge()) {
        return EventHandler::markdownBody(&**event);
    }
    return {};
}

MessageContentModel *ChatBarCache::relationEventContentModel()
{
    if (parent() == nullptr) {
        qWarning() << "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.";
        return nullptr;
    }
    if (m_relationId.isEmpty()) {
        return nullptr;
    }
    if (m_relationContentModel != nullptr) {
        return m_relationContentModel;
    }

    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.";
        return nullptr;
    }
    m_relationContentModel = new MessageContentModel(room, m_relationId, true);
    return m_relationContentModel;
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
    const auto oldEventId = std::exchange(m_relationId, QString());
    delete m_relationContentModel;
    Q_EMIT attachmentPathChanged();
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
}

void ChatBarCache::clearRelations()
{
    const auto oldEventId = std::exchange(m_relationId, QString());
    const auto oldThreadId = std::exchange(m_threadId, QString());
    m_attachmentPath = QString();
    delete m_relationContentModel;
    Q_EMIT relationIdChanged(oldEventId, m_relationId);
    Q_EMIT threadIdChanged(oldThreadId, m_threadId);
    Q_EMIT attachmentPathChanged();
}

QList<Mention> *ChatBarCache::mentions()
{
    return &m_mentions;
}

void ChatBarCache::updateMentions(QQuickTextDocument *document, ChatDocumentHandler *documentHandler)
{
    documentHandler->setDocument(document);

    if (parent() == nullptr) {
        qWarning() << "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.";
        return;
    }
    if (m_relationId.isEmpty()) {
        return;
    }
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.";
        return;
    }

    if (auto event = room->findInTimeline(m_relationId); event != room->historyEdge()) {
        if (const auto &roomMessageEvent = &*event->viewAs<Quotient::RoomMessageEvent>()) {
            // Replaces the mentions that are baked into the HTML but plaintext in the original markdown
            const QRegularExpression re(QStringLiteral(R"lit(<a\shref="https:\/\/matrix.to\/#\/([\S]*)"\s?>([\S]*)<\/a>)lit"));

            m_mentions.clear();

            int linkSize = 0;
            auto matches = re.globalMatch(EventHandler::rawMessageBody(*roomMessageEvent));
            while (matches.hasNext()) {
                const QRegularExpressionMatch match = matches.next();
                if (match.hasMatch()) {
                    const QString id = match.captured(1);
                    const QString name = match.captured(2);

                    const int position = match.capturedStart(0) - linkSize;
                    const int end = position + name.length();
                    linkSize += match.capturedLength(0) - name.length();

                    QTextCursor cursor(documentHandler->document()->textDocument());
                    cursor.setPosition(position);
                    cursor.setPosition(end, QTextCursor::KeepAnchor);
                    cursor.setKeepPositionOnInsert(true);

                    m_mentions.push_back(Mention{.cursor = cursor, .text = name, .start = position, .position = end, .id = id});
                }
            }
        }
    }
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
    auto room = dynamic_cast<NeoChatRoom *>(parent());
    if (room == nullptr) {
        qWarning() << "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.";
        return;
    }

    if (!attachmentPath().isEmpty()) {
        room->uploadFile(QUrl(attachmentPath()), sendText());
        clearCache();
        return;
    }

    const auto result = ActionsModel::handleAction(room, this);
    if (!result.first.has_value()) {
        return;
    }

    TextHandler textHandler;
    textHandler.setData(*std::get<std::optional<QString>>(result));
    const auto sendText = textHandler.handleSendText();

    if (sendText.length() == 0) {
        return;
    }

    room->postMessage(text(), sendText, *std::get<std::optional<Quotient::RoomMessageEvent::MsgType>>(result), replyId(), editId(), threadId());
    clearCache();
}

void ChatBarCache::clearCache()
{
    setText({});
    m_mentions.clear();
    m_savedText = QString();
    clearRelations();
}

#include "moc_chatbarcache.cpp"
