// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include <QTextDocumentFragment>

#include "chatbarcache.h"
#include "chatdocumenthandler.h"
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
    m_components += MessageComponent{
        .type = MessageComponentType::Text,
        .display = {},
        .attributes = {{"chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(new ChatDocumentHandler())}},
    };
    m_currentFocusComponent = QPersistentModelIndex(index(0));
    endInsertRows();
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

int ChatBarMessageContentModel::focusRow() const
{
    return m_currentFocusComponent.row();
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
            chatDocumentHandler->disconnect(this);
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

void ChatBarMessageContentModel::addNewComponent(MessageComponentType::Type type)
{
    if (m_components.last().type == type) {
        return;
    }

    if (const auto chatDocumentHandler = focusedDocumentHandler(); chatDocumentHandler && chatDocumentHandler->isEmpty() && m_components.size() <= 1) {
        removeComponent(0, true);
    }

    insertComponent(rowCount(), type);
}

ChatBarMessageContentModel::ComponentIt ChatBarMessageContentModel::insertComponent(int row, MessageComponentType::Type type)
{
    if (row < 0 || row > rowCount()) {
        return m_components.end();
    }

    QVariantMap attributes;
    if (MessageComponentType::isTextType(type)) {
        attributes.insert("chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(new ChatDocumentHandler()));
    }
    beginInsertRows({}, row, row);
    const auto it = m_components.insert(row,
                                        MessageComponent{
                                            .type = type,
                                            .display = {},
                                            .attributes = attributes,
                                        });
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
        m_components[row].attributes.remove("chatDocumentHandler"_L1);
        chatDocumentHandler->deleteLater();
    }
    m_components.removeAt(row);
    endRemoveRows();
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

QString ChatBarMessageContentModel::messageText() const
{
    QString text;
    for (const auto &component : m_components) {
        if (MessageComponentType::isTextType(component.type)) {
            if (const auto chatDocumentHandler = documentHandlerForComponent(component)) {
                auto newText = chatDocumentHandler->htmlText();
                if (component.type == MessageComponentType::Quote) {
                    newText = formatQuote(newText);
                }
                text += newText;
            }
        }
    }
    return text;
}

void ChatBarMessageContentModel::postMessage()
{
    qWarning() << "post message" << messageText();
}

std::optional<QString> ChatBarMessageContentModel::getReplyEventId()
{
    return m_room->mainCache()->isReplying() ? std::make_optional(m_room->mainCache()->replyId()) : std::nullopt;
}

#include "moc_chatbarmessagecontentmodel.cpp"
