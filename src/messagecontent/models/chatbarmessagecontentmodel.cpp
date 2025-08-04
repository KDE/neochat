// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include "chatbarcache.h"
#include "chatdocumenthandler.h"
#include "enums/messagecomponenttype.h"
#include <qcontainerfwd.h>
#include <qvariant.h>

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
        .display = u"1"_s,
        .attributes = {{"chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(new ChatDocumentHandler())}},
    };
    m_components += MessageComponent{
        .type = MessageComponentType::Text,
        .display = u"2"_s,
        .attributes = {{"chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(new ChatDocumentHandler())}},
    };
    endInsertRows();
}

int ChatBarMessageContentModel::focusRow() const
{
    return m_currentFocusComponent.row();
}

void ChatBarMessageContentModel::setFocusRow(int focusRow)
{
    if (focusRow == m_currentFocusComponent.row()) {
        return;
    }

    if (m_currentFocusComponent.isValid()) {
        if (const auto chatDocumentHandler =
                qvariant_cast<ChatDocumentHandler *>(m_components[m_currentFocusComponent.row()].attributes["chatDocumentHandler"_L1])) {
            chatDocumentHandler->disconnect(this);
        }
    }

    if (focusRow < 0 || focusRow >= rowCount()) {
        m_currentFocusComponent = QPersistentModelIndex();
    } else {
        m_currentFocusComponent = QPersistentModelIndex(index(focusRow));
    }

    if (m_currentFocusComponent.isValid()) {
        if (const auto chatDocumentHandler =
                qvariant_cast<ChatDocumentHandler *>(m_components[m_currentFocusComponent.row()].attributes["chatDocumentHandler"_L1])) {
            connect(chatDocumentHandler, &ChatDocumentHandler::boldChanged, this, &ChatBarMessageContentModel::boldChanged);
        }
    }

    Q_EMIT focusRowChanged();
    emitFocusChangeSignals();
}

void ChatBarMessageContentModel::emitFocusChangeSignals()
{
    Q_EMIT boldChanged();
}

bool ChatBarMessageContentModel::bold() const
{
    if (!m_currentFocusComponent.isValid()) {
        return false;
    }

    if (const auto chatDocumentHandler =
            qvariant_cast<ChatDocumentHandler *>(m_components[m_currentFocusComponent.row()].attributes["chatDocumentHandler"_L1])) {
        return chatDocumentHandler->bold();
    }
    return false;
}

void ChatBarMessageContentModel::setBold(bool bold)
{
    if (!m_currentFocusComponent.isValid()) {
        return;
    }

    if (const auto chatDocumentHandler =
            qvariant_cast<ChatDocumentHandler *>(m_components[m_currentFocusComponent.row()].attributes["chatDocumentHandler"_L1])) {
        return chatDocumentHandler->setBold(bold);
    }
}

void ChatBarMessageContentModel::addNewComponent(MessageComponentType::Type type)
{
    QVariantMap attributes;
    if (MessageComponentType::isTextType(type)) {
        attributes.insert("chatDocumentHandler"_L1, QVariant::fromValue<ChatDocumentHandler *>(new ChatDocumentHandler()));
    }

    beginInsertRows({}, rowCount(), rowCount());
    m_components += MessageComponent{
        .type = type,
        .display = {},
        .attributes = attributes,
    };
    endInsertRows();
}

void ChatBarMessageContentModel::postMessage()
{
    return;
}

std::optional<QString> ChatBarMessageContentModel::getReplyEventId()
{
    return m_room->mainCache()->isReplying() ? std::make_optional(m_room->mainCache()->replyId()) : std::nullopt;
}

#include "moc_chatbarmessagecontentmodel.cpp"
