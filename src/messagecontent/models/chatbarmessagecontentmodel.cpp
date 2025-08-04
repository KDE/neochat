// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include <QTextDocumentFragment>

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
        if (m_type == ChatBarType::None || !m_room) {
            return;
        }

        connect(m_room->cacheForType(m_type), &ChatBarCache::relationIdChanged, this, &ChatBarMessageContentModel::updateReplyModel);
        clearModel();

        beginResetModel();

        if (m_room->cacheForType(m_type)->attachmentPath().length() > 0) {
            addAttachment(QUrl(m_room->cacheForType(m_type)->attachmentPath()));
        }

        const auto textSections = m_room->cacheForType(m_type)->text().split(u"\n\n"_s);
        for (const auto &section : textSections) {
            const auto type = MessageComponentType::typeForString(section);
            auto cleanText = section;
            if (type == MessageComponentType::Code) {
                cleanText.remove(0, 4);
                cleanText.remove(cleanText.length() - 4, 4);
            } else if (type == MessageComponentType::Quote) {
                cleanText.remove(0, 2);
            }
            insertComponent(rowCount(), type, {}, cleanText);
        }
        m_currentFocusComponent = QPersistentModelIndex(index(rowCount() - 1));
        endResetModel();

        Q_EMIT focusRowChanged();
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

    Q_EMIT focusRowChanged();
}

void ChatBarMessageContentModel::connectHandler(ChatDocumentHandler *handler)
{
    connect(handler, &ChatDocumentHandler::contentsChanged, this, &ChatBarMessageContentModel::updateCache);
    connect(handler, &ChatDocumentHandler::unhandledBackspaceAtBeginning, this, [this](ChatDocumentHandler *handler) {
        const auto index = indexForDocumentHandler(handler);
        if (index.isValid()) {
            if (index.row() > 0 && MessageComponentType::isFileType(m_components[index.row() - 1].type)) {
                removeAttachment();
            } else if (m_components[index.row()].type == MessageComponentType::Code || m_components[index.row()].type == MessageComponentType::Quote) {
                insertComponentAtCursor(MessageComponentType::Text);
            }
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
    const auto oldIndex = std::exchange(m_currentFocusComponent, QPersistentModelIndex(index));

    if (m_currentFocusComponent.isValid()) {
        if (!mouse) {
            focusCurrentComponent(oldIndex, m_currentFocusComponent.row() > oldIndex.row());
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
    if (!m_currentFocusComponent.isValid()) {
        return nullptr;
    }

    if (const auto chatDocumentHandler = documentHandlerForIndex(m_currentFocusComponent)) {
        return chatDocumentHandler;
    }
    return nullptr;
}

void ChatBarMessageContentModel::emitFocusChangeSignals()
{
    Q_EMIT dataChanged(index(0), index(rowCount() - 1), {CurrentFocusRole});
}

void ChatBarMessageContentModel::addAttachment(const QUrl &path)
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    auto it = insertComponent(m_components.first().type == MessageComponentType::Reply ? 1 : 0,
                              MessageComponentType::typeForPath(path),
                              {
                                  {"filename"_L1, path.fileName()},
                                  {"source"_L1, path},
                                  {"animated"_L1, false},
                              });
    it->display = path.fileName();
    ++it;
    Q_EMIT dataChanged(index(std::distance(m_components.begin(), it)), index(std::distance(m_components.begin(), it)), {DisplayRole});

    bool textKept = false;
    while (it != m_components.end()) {
        if (it->type != MessageComponentType::Text || textKept) {
            it = removeComponent(it);
        } else {
            textKept = true;
            ++it;
        }
    }

    m_room->cacheForType(m_type)->setAttachmentPath(path.toString());
}

ChatBarMessageContentModel::ComponentIt
ChatBarMessageContentModel::insertComponent(int row, MessageComponentType::Type type, QVariantMap attributes, const QString &intialText)
{
    if (row < 0 || row > rowCount()) {
        return m_components.end();
    }

    if (MessageComponentType::isTextType(type)) {
        const auto documentHandler = new ChatDocumentHandler();
        documentHandler->setInitialText(intialText);
        if (type == MessageComponentType::Quote) {
            documentHandler->setFixedStartChars(u"\""_s);
            documentHandler->setFixedEndChars(u"\""_s);
        }

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
        if (type == MessageComponentType::Text && focusedDocumentHandler()) {
            focusedDocumentHandler()->setStyle(ChatDocumentHandler::Paragraph);
        }
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
        Q_EMIT focusRowChanged();
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

    removeComponent(m_components.begin() + row);
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

ChatBarMessageContentModel::ComponentIt ChatBarMessageContentModel::removeComponent(ComponentIt it)
{
    if (it == m_components.end()) {
        return it;
    }

    const auto row = std::distance(m_components.begin(), it);
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
        const auto afterHandler = chatDocumentHandler->nextDocumentHandler();
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
    it = m_components.erase(it);
    endRemoveRows();

    return it;
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

    qWarning() << m_room->cacheForType(m_type)->text();
    m_room->cacheForType(m_type)->postMessage();
    clearModel();
    initializeModel();
}

std::optional<QString> ChatBarMessageContentModel::getReplyEventId()
{
    return m_room->mainCache()->isReplying() ? std::make_optional(m_room->mainCache()->replyId()) : std::nullopt;
}

void ChatBarMessageContentModel::clearModel()
{
    beginResetModel();
    for (const auto &component : m_components) {
        if (const auto chatDocumentHandler = documentHandlerForComponent(component)) {
            chatDocumentHandler->disconnect(this);
            chatDocumentHandler->deleteLater();
        }
    }
    m_components.clear();
    endResetModel();
}

#include "moc_chatbarmessagecontentmodel.cpp"
