// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include <QTextDocumentFragment>
#include <asm-generic/errno.h>

#include "chatbarcache.h"
#include "chatdocumenthandler.h"
#include "enums/chatbartype.h"
#include "enums/messagecomponenttype.h"
#include "messagecontentmodel.h"

ChatBarMessageContentModel::ChatBarMessageContentModel(QObject *parent)
    : MessageContentModel(parent)
{
    m_editableActive = true;
    initializeModel();

    connect(this, &ChatBarMessageContentModel::roomChanged, this, [this]() {
        if (!m_room) {
            return;
        }

        connect(m_room->mainCache(), &ChatBarCache::relationIdChanged, this, &ChatBarMessageContentModel::updateReplyModel);
    });
}

void ChatBarMessageContentModel::initializeModel()
{
    beginInsertRows({}, rowCount(), rowCount());
    const auto documentHandler = new ChatDocumentHandler();
    connectHandler(documentHandler);
    m_components += MessageComponent{
        .type = MessageComponentType::Text,
        .display = {},
        .attributes = {{"chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(documentHandler)}},
    };
    m_currentFocusComponent = QPersistentModelIndex(index(0));
    endInsertRows();
}

void ChatBarMessageContentModel::connectHandler(ChatDocumentHandler *handler)
{
    connect(handler, &ChatDocumentHandler::contentsChanged, this, &ChatBarMessageContentModel::updateCache);
    connect(handler, &ChatDocumentHandler::unhandledBackspaceAtBeginning, this, [this](ChatDocumentHandler *handler) {
        const auto index = indexForDocumentHandler(handler);
        if (index.isValid() && index.row() > 0 && MessageComponentType::isFileType(m_components[index.row() - 1].type)) {
            removeAttachment();
        }
    });
    connect(handler, &ChatDocumentHandler::removeMe, this, [this](ChatDocumentHandler *handler) {
        removeComponent(handler);
    });
}

ChatDocumentHandler *ChatBarMessageContentModel::documentHandlerForComponent(const MessageComponent &component) const
{
    if (const auto chatDocumentHandler = qvariant_cast<ChatDocumentHandler *>(component.attributes["chatDocumentHandler"_L1])) {
        return chatDocumentHandler;
    }
    return nullptr;
}

ChatDocumentHandler *ChatBarMessageContentModel::documentHandlerForIndex(const QModelIndex &index) const
{
    return documentHandlerForComponent(m_components[index.row()]);
}

QModelIndex ChatBarMessageContentModel::indexForDocumentHandler(ChatDocumentHandler *handler) const
{
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        const auto currentIndex = index(it - m_components.begin());
        if (documentHandlerForIndex(currentIndex) == handler) {
            return currentIndex;
        }
    }
    return {};
}

void ChatBarMessageContentModel::updateDocumentHandlerRefs(const ComponentIt &it)
{
    if (it == m_components.end()) {
        return;
    }

    const auto handler = documentHandlerForComponent(*it);
    if (!handler) {
        return;
    }

    if (it != m_components.begin()) {
        if (const auto beforeHandler = documentHandlerForComponent(*(it - 1))) {
            beforeHandler->setNextDocumentHandler(handler);
            handler->setPreviousDocumentHandler(beforeHandler);
        }
    }
    if (it + 1 != m_components.end()) {
        if (const auto afterHandler = documentHandlerForComponent(*(it + 1))) {
            afterHandler->setPreviousDocumentHandler(handler);
            handler->setNextDocumentHandler(afterHandler);
        }
    }
}

ChatBarType::Type ChatBarMessageContentModel::type() const
{
    return m_type;
}

void ChatBarMessageContentModel::setType(ChatBarType::Type type)
{
    if (type == m_type) {
        return;
    }
    m_type = type;
    Q_EMIT typeChanged();
}

int ChatBarMessageContentModel::focusRow() const
{
    return m_currentFocusComponent.row();
}

MessageComponentType::Type ChatBarMessageContentModel::focusType() const
{
    return static_cast<MessageComponentType::Type>(m_currentFocusComponent.data(ComponentTypeRole).toInt());
}

void ChatBarMessageContentModel::setFocusRow(int focusRow, bool mouse)
{
    if (focusRow == m_currentFocusComponent.row() || focusRow < 0 || focusRow >= rowCount()) {
        return;
    }

    setFocusIndex(index(focusRow), mouse);
}

void ChatBarMessageContentModel::setFocusIndex(const QModelIndex &index, bool mouse)
{
    if (m_currentFocusComponent.isValid()) {
        if (const auto chatDocumentHandler = focusedDocumentHandler()) {
            disconnect(chatDocumentHandler, &ChatDocumentHandler::boldChanged, nullptr, nullptr);
        }
    }

    const auto oldIndex = std::exchange(m_currentFocusComponent, QPersistentModelIndex(index));

    if (m_currentFocusComponent.isValid()) {
        if (!mouse) {
            focusCurrentComponent(oldIndex, m_currentFocusComponent.row() > oldIndex.row());
        }
        if (const auto chatDocumentHandler = focusedDocumentHandler()) {
            connect(chatDocumentHandler, &ChatDocumentHandler::boldChanged, this, &ChatBarMessageContentModel::boldChanged);
        }
    }

    Q_EMIT focusRowChanged();
    emitFocusChangeSignals();
}

void ChatBarMessageContentModel::focusCurrentComponent(const QModelIndex &previousIndex, bool down)
{
    const auto chatDocumentHandler = focusedDocumentHandler();
    if (!chatDocumentHandler) {
        return;
    }

    chatDocumentHandler->setCursorFromDocumentHandler(documentHandlerForIndex(previousIndex), down, MessageComponentType::Quote ? 1 : 0);
}

void ChatBarMessageContentModel::refocusCurrentComponent() const
{
    const auto chatDocumentHandler = focusedDocumentHandler();
    if (!chatDocumentHandler) {
        return;
    }

    chatDocumentHandler->textItem()->forceActiveFocus();
}

ChatDocumentHandler *ChatBarMessageContentModel::focusedDocumentHandler() const
{
    if (const auto chatDocumentHandler = documentHandlerForIndex(m_currentFocusComponent)) {
        return chatDocumentHandler;
    }
    return nullptr;
}

void ChatBarMessageContentModel::emitFocusChangeSignals()
{
    Q_EMIT dataChanged(index(0), index(rowCount() - 1), {CurrentFocusRole});
    Q_EMIT boldChanged();
    Q_EMIT italicChanged();
}

bool ChatBarMessageContentModel::bold() const
{
    if (!m_currentFocusComponent.isValid()) {
        return false;
    }

    if (const auto chatDocumentHandler = focusedDocumentHandler()) {
        return chatDocumentHandler->bold();
    }
    return false;
}

void ChatBarMessageContentModel::setBold(bool bold)
{
    if (!m_currentFocusComponent.isValid()) {
        return;
    }

    if (const auto chatDocumentHandler = focusedDocumentHandler()) {
        return chatDocumentHandler->setBold(bold);
    }
}

bool ChatBarMessageContentModel::italic() const
{
    if (!m_currentFocusComponent.isValid()) {
        return false;
    }

    if (const auto chatDocumentHandler = focusedDocumentHandler()) {
        return chatDocumentHandler->italic();
    }
    return false;
}

void ChatBarMessageContentModel::setItalic(bool italic)
{
    if (!m_currentFocusComponent.isValid()) {
        return;
    }

    if (const auto chatDocumentHandler = focusedDocumentHandler()) {
        return chatDocumentHandler->setItalic(italic);
    }
}

void ChatBarMessageContentModel::addAttachment(const QUrl &path)
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    insertComponent(m_components.first().type == MessageComponentType::Reply ? 1 : 0,
                    MessageComponentType::typeForPath(path),
                    {
                        {"filename"_L1, path.fileName()},
                        {"source"_L1, path},
                        {"animated"_L1, false},
                    });
    m_room->cacheForType(m_type)->setAttachmentPath(path.toString());
}

ChatBarMessageContentModel::ComponentIt ChatBarMessageContentModel::insertComponent(int row, MessageComponentType::Type type, QVariantMap attributes)
{
    if (row < 0 || row > rowCount()) {
        return m_components.end();
    }

    if (MessageComponentType::isTextType(type)) {
        const auto documentHandler = new ChatDocumentHandler();
        attributes.insert("chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(documentHandler));
        connectHandler(documentHandler);
    }
    beginInsertRows({}, row, row);
    const auto it = m_components.insert(row,
                                        MessageComponent{
                                            .type = type,
                                            .display = {},
                                            .attributes = attributes,
                                        });
    updateDocumentHandlerRefs(it);
    endInsertRows();
    return it;
}

void ChatBarMessageContentModel::insertComponentAtCursor(MessageComponentType::Type type)
{
    if (m_components[m_currentFocusComponent.row()].type == type) {
        return;
    }

    bool hasBefore = false;
    QTextDocumentFragment midFragment;
    std::optional<QTextDocumentFragment> afterFragment = std::nullopt;

    if (const auto currentChatDocumentHandler = focusedDocumentHandler()) {
        currentChatDocumentHandler->fillFragments(hasBefore, midFragment, afterFragment);
    }

    const auto currentType = m_components[m_currentFocusComponent.row()].type;
    int insertRow = m_currentFocusComponent.row() + (hasBefore ? 1 : 0);

    if (!hasBefore) {
        removeComponent(insertRow, true);
    }

    const auto insertIt = insertComponent(insertRow, type);
    if (insertIt != m_components.end()) {
        if (const auto insertChatDocumentHandler = documentHandlerForComponent(*insertIt)) {
            insertChatDocumentHandler->insertFragment(midFragment);
        }
        m_currentFocusComponent = QPersistentModelIndex(index(insertIt - m_components.begin()));
        emitFocusChangeSignals();
    }

    if (afterFragment) {
        const auto afterIt = insertComponent(insertRow + 1, currentType);
        if (afterIt != m_components.end()) {
            if (const auto afterChatDocumentHandler = documentHandlerForComponent(*afterIt)) {
                afterChatDocumentHandler->insertFragment(*afterFragment);
            }
        }
    }
}

void ChatBarMessageContentModel::removeComponent(int row, bool removeLast)
{
    if (row < 0 || row >= rowCount() || (rowCount() == 1 && !removeLast)) {
        return;
    }

    beginRemoveRows({}, row, row);
    if (rowCount() == 1) {
        setFocusIndex({});
    } else if (m_currentFocusComponent.row() == row) {
        int newFocusRow;
        if (row > 0) {
            newFocusRow = row - 1;
        } else {
            newFocusRow = row + 1;
        }
        setFocusRow(newFocusRow);
    }

    if (const auto chatDocumentHandler = documentHandlerForIndex(index(row))) {
        const auto beforeHandler = chatDocumentHandler->previousDocumentHandler();
        const auto afterHandler = chatDocumentHandler->previousDocumentHandler();
        if (beforeHandler && afterHandler) {
            beforeHandler->setNextDocumentHandler(afterHandler);
            afterHandler->setPreviousDocumentHandler(beforeHandler);
        } else if (beforeHandler) {
            beforeHandler->setNextDocumentHandler(nullptr);
        } else if (afterHandler) {
            afterHandler->setPreviousDocumentHandler(nullptr);
        }

        m_components[row].attributes.remove("chatDocumentHandler"_L1);
        chatDocumentHandler->disconnect(this);
        chatDocumentHandler->deleteLater();
    }
    m_components.removeAt(row);
    endRemoveRows();
}

void ChatBarMessageContentModel::removeAttachment()
{
    if (!hasComponentType({MessageComponentType::File, MessageComponentType::Audio, MessageComponentType::Image, MessageComponentType::Video})) {
        return;
    }

    auto mediaRow = 0;
    if (MessageComponentType::isFileType(m_components[1].type)) {
        mediaRow = 1;
    }
    removeComponent(mediaRow);
    if (m_room) {
        m_room->cacheForType(m_type)->setAttachmentPath({});
    }
}

void ChatBarMessageContentModel::removeComponent(ChatDocumentHandler *handler)
{
    const auto index = indexForDocumentHandler(handler);
    if (index.isValid()) {
        removeComponent(index.row());
    }
}

void ChatBarMessageContentModel::updateCache() const
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    m_room->cacheForType(m_type)->setText(messageText());
}

inline QString formatQuote(const QString &input)
{
    QString stringOut;
    auto splitString = input.split(u"\n\n"_s, Qt::SkipEmptyParts);
    for (auto &string : splitString) {
        if (string.startsWith(u'*')) {
            string.removeFirst();
        }
        if (string.startsWith(u'\"')) {
            string.removeFirst();
        }
        if (string.endsWith(u'*')) {
            string.removeLast();
        }
        if (string.endsWith(u'\"')) {
            string.removeLast();
        }
        if (!stringOut.isEmpty()) {
            stringOut += u"\n\n"_s;
        }
        stringOut += u"> "_s + string;
    }
    return stringOut;
}

inline QString formatCode(const QString &input)
{
    return u"```\n%1\n```"_s.arg(input).replace(u"\n\n"_s, u"\n"_s);
}

QString ChatBarMessageContentModel::messageText() const
{
    QString text;
    for (const auto &component : m_components) {
        if (MessageComponentType::isTextType(component.type)) {
            if (const auto chatDocumentHandler = documentHandlerForComponent(component)) {
                auto newText = chatDocumentHandler->htmlText();
                if (component.type == MessageComponentType::Quote) {
                    newText = formatQuote(newText);
                } else if (component.type == MessageComponentType::Code) {
                    newText = formatCode(newText);
                }
                if (!text.isEmpty()) {
                    text += u"\n\n"_s;
                }
                text += newText;
            }
        }
    }
    return text;
}

void ChatBarMessageContentModel::postMessage()
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    m_room->cacheForType(m_type)->postMessage();
}

std::optional<QString> ChatBarMessageContentModel::getReplyEventId()
{
    return m_room->mainCache()->isReplying() ? std::make_optional(m_room->mainCache()->replyId()) : std::nullopt;
}

#include "moc_chatbarmessagecontentmodel.cpp"
