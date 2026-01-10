// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include <QTextDocumentFragment>
#include <qlogging.h>

#include "chatbarcache.h"
#include "chatkeyhelper.h"
#include "chatmarkdownhelper.h"
#include "chattextitemhelper.h"
#include "enums/chatbartype.h"
#include "enums/messagecomponenttype.h"
#include "enums/richformat.h"
#include "messagecontentmodel.h"

namespace
{
constexpr auto TextItemKey = "chatTextItemHelper"_L1;
}

ChatBarMessageContentModel::ChatBarMessageContentModel(QObject *parent)
    : MessageContentModel(parent)
    , m_markdownHelper(new ChatMarkdownHelper(this))
    , m_keyHelper(new ChatKeyHelper(this))
{
    m_editableActive = true;

    connect(this, &ChatBarMessageContentModel::roomChanged, this, [this]() {
        if (m_type == ChatBarType::None || !m_room) {
            return;
        }

        connect(m_room->cacheForType(m_type), &ChatBarCache::relationIdChanged, this, &ChatBarMessageContentModel::updateReplyModel);
        connect(m_room->cacheForType(m_type), &ChatBarCache::attachmentPathChanged, this, [this]() {
            if (m_room->cacheForType(m_type)->attachmentPath().length() > 0) {
                addAttachment(QUrl(m_room->cacheForType(m_type)->attachmentPath()));
            }
        });

        if (m_room->cacheForType(m_type)->attachmentPath().length() > 0) {
            addAttachment(QUrl(m_room->cacheForType(m_type)->attachmentPath()));
        }

        const auto textSections = m_room->cacheForType(m_type)->text().split(u"\n\n"_s);
        if (textSections.length() == 1 && textSections[0].isEmpty()) {
            return;
        }

        clearModel();
        beginResetModel();
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
        endResetModel();

        m_currentFocusComponent = QPersistentModelIndex(index(rowCount() - 1));
        Q_EMIT focusRowChanged();
    });
    connect(this, &ChatBarMessageContentModel::focusRowChanged, this, [this]() {
        m_markdownHelper->setTextItem(focusedTextItem());
        m_keyHelper->textItem = focusedTextItem();
    });
    connect(this, &ChatBarMessageContentModel::roomChanged, this, [this]() {
        for (const auto &component : m_components) {
            if (const auto textItem = textItemForComponent(component)) {
                textItem->setRoom(m_room);
            }
        }
        m_keyHelper->room = m_room;
    });
    connect(this, &ChatBarMessageContentModel::typeChanged, this, [this]() {
        for (const auto &component : m_components) {
            if (const auto textItem = textItemForComponent(component)) {
                textItem->setType(m_type);
            }
        }
    });
    connect(m_markdownHelper, &ChatMarkdownHelper::unhandledBlockFormat, this, &ChatBarMessageContentModel::insertStyleAtCursor);

    connectKeyHelper();
    initializeModel();
}

void ChatBarMessageContentModel::initializeModel(const QString &initialText)
{
    updateReplyModel();

    beginInsertRows({}, rowCount(), rowCount());
    const auto textItem = new ChatTextItemHelper(this);
    textItem->setRoom(m_room);
    textItem->setType(m_type);
    textItem->setInitialText(initialText);
    connectTextItem(textItem);
    m_components += MessageComponent{
        .type = MessageComponentType::Text,
        .display = {},
        .attributes = {{TextItemKey, QVariant::fromValue<ChatTextItemHelper *>(textItem)}},
    };
    endInsertRows();

    m_currentFocusComponent = QPersistentModelIndex(index(0));
    Q_EMIT focusRowChanged();
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

ChatKeyHelper *ChatBarMessageContentModel::keyHelper() const
{
    return m_keyHelper;
}

void ChatBarMessageContentModel::connectKeyHelper()
{
    connect(m_keyHelper, &ChatKeyHelper::unhandledUp, this, [this](bool isCompleting) {
        if (isCompleting) {
            return;
        }
        if (!m_room->editCache()->isEditing() && m_currentFocusComponent.row() <= 0 && focusedTextItem()->isEmpty()) {
            m_room->editLastMessage();
            return;
        }
        handleBlockTransition(true);
    });
    connect(m_keyHelper, &ChatKeyHelper::unhandledDown, this, [this](bool isCompleting) {
        if (isCompleting) {
            return;
        }
        handleBlockTransition(false);
    });
    connect(m_keyHelper, &ChatKeyHelper::unhandledDelete, this, [this]() {
        const auto currentRow = m_currentFocusComponent.row();
        if (currentRow < m_components.size() - 1) {
            if (const auto nextTextItem = textItemForComponent(m_components[currentRow + 1])) {
                focusedTextItem()->insertFragment(nextTextItem->takeFirstBlock(), ChatTextItemHelper::Cursor, true);
            }
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::unhandledBackspace, this, [this]() {
        const auto currentRow = m_currentFocusComponent.row();
        if (currentRow > 0) {
            const auto previousRow = currentRow - 1;
            if (MessageComponentType::isFileType(m_components[previousRow].type)) {
                removeAttachment();
                return;
            }
            if (const auto previousTextItem = textItemForComponent(m_components[previousRow])) {
                previousTextItem->insertFragment(focusedTextItem()->takeFirstBlock(), ChatTextItemHelper::End, true);
                return;
            }
        }
        if (m_components[currentRow].type == MessageComponentType::Code || m_components[currentRow].type == MessageComponentType::Quote) {
            insertComponentAtCursor(MessageComponentType::Text);
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::imagePasted, this, [this](const QString &filePath) {
        m_room->cacheForType(m_type)->setAttachmentPath(filePath);
    });
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

    emitFocusChangeSignals();
}

void ChatBarMessageContentModel::focusCurrentComponent(const QModelIndex &previousIndex, bool down)
{
    const auto textItem = focusedTextItem();
    if (!textItem) {
        return;
    }

    textItem->setCursorFromTextItem(textItemForIndex(previousIndex), down);
}

void ChatBarMessageContentModel::refocusCurrentComponent() const
{
    const auto textItem = focusedTextItem();
    if (!textItem) {
        return;
    }

    textItem->forceActiveFocus();
}

void ChatBarMessageContentModel::emitFocusChangeSignals()
{
    Q_EMIT focusRowChanged();
    Q_EMIT dataChanged(index(0), index(rowCount() - 1), {CurrentFocusRole});
}

ChatTextItemHelper *ChatBarMessageContentModel::focusedTextItem() const
{
    if (!m_currentFocusComponent.isValid()) {
        return nullptr;
    }
    if (const auto textItem = textItemForIndex(m_currentFocusComponent)) {
        return textItem;
    }
    return nullptr;
}

void ChatBarMessageContentModel::connectTextItem(ChatTextItemHelper *chattextitemhelper)
{
    connect(chattextitemhelper, &ChatTextItemHelper::contentsChanged, this, &ChatBarMessageContentModel::updateCache);
    connect(chattextitemhelper, &ChatTextItemHelper::contentsChanged, this, &ChatBarMessageContentModel::hasRichFormattingChanged);
    connect(chattextitemhelper, &ChatTextItemHelper::charFormatChanged, this, &ChatBarMessageContentModel::hasRichFormattingChanged);
    connect(chattextitemhelper, &ChatTextItemHelper::styleChanged, this, &ChatBarMessageContentModel::hasRichFormattingChanged);
    connect(chattextitemhelper, &ChatTextItemHelper::listChanged, this, &ChatBarMessageContentModel::hasRichFormattingChanged);
    connect(chattextitemhelper, &ChatTextItemHelper::cleared, this, [this](ChatTextItemHelper *helper) {
        removeComponent(helper);
    });
}

ChatTextItemHelper *ChatBarMessageContentModel::textItemForComponent(const MessageComponent &component) const
{
    if (const auto textItem = qvariant_cast<ChatTextItemHelper *>(component.attributes[TextItemKey])) {
        return textItem;
    }
    return nullptr;
}

ChatTextItemHelper *ChatBarMessageContentModel::textItemForIndex(const QModelIndex &index) const
{
    return textItemForComponent(m_components[index.row()]);
}

QModelIndex ChatBarMessageContentModel::indexForTextItem(ChatTextItemHelper *textItem) const
{
    if (!textItem) {
        return {};
    }

    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        const auto currentIndex = index(it - m_components.begin());
        if (textItemForIndex(currentIndex) == textItem) {
            return currentIndex;
        }
    }
    return {};
}

bool ChatBarMessageContentModel::hasRichFormatting() const
{
    for (const auto &component : m_components) {
        if (component.type != MessageComponentType::Text) {
            return true;
        }
        if (const auto textItem = textItemForComponent(component)) {
            if (textItem->hasRichFormatting()) {
                return true;
            }
        }
    }
    return false;
}

void ChatBarMessageContentModel::addAttachment(const QUrl &path)
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    QString plainText;
    for (const auto &component : m_components) {
        if (const auto textItem = textItemForComponent(component)) {
            plainText += u"%1%2"_s.arg(plainText.isEmpty() ? u""_s : u"\n"_s, textItem->plainText());
        }
    }

    clearModel();
    initializeModel(plainText);

    auto it = insertComponent(m_components.first().type == MessageComponentType::Reply ? 1 : 0,
                              MessageComponentType::typeForPath(path),
                              {
                                  {"filename"_L1, path.fileName()},
                                  {"source"_L1, path},
                                  {"animated"_L1, false},
                              });
    it->display = path.fileName();
    Q_EMIT dataChanged(index(std::distance(m_components.begin(), it)), index(std::distance(m_components.begin(), it)), {DisplayRole});
    m_room->cacheForType(m_type)->setAttachmentPath(path.toString());
}

ChatBarMessageContentModel::ComponentIt
ChatBarMessageContentModel::insertComponent(int row, MessageComponentType::Type type, QVariantMap attributes, const QString &intialText)
{
    if (row < 0 || row > rowCount()) {
        return m_components.end();
    }

    if (MessageComponentType::isTextType(type)) {
        const auto textItemWrapper = new ChatTextItemHelper(this);
        textItemWrapper->setInitialText(intialText);
        textItemWrapper->setRoom(m_room);
        textItemWrapper->setType(m_type);
        if (type == MessageComponentType::Quote) {
            textItemWrapper->setFixedChars(u"\""_s, u"\""_s);
        }

        attributes.insert(TextItemKey, QVariant::fromValue<ChatTextItemHelper *>(textItemWrapper));
        connectTextItem(textItemWrapper);
    }
    beginInsertRows({}, row, row);
    const auto it = m_components.insert(row,
                                        MessageComponent{
                                            .type = type,
                                            .display = {},
                                            .attributes = attributes,
                                        });
    endInsertRows();
    Q_EMIT hasRichFormattingChanged();

    return it;
}

void ChatBarMessageContentModel::insertStyleAtCursor(RichFormat::Format style)
{
    switch (style) {
    case RichFormat::Paragraph:
        insertComponentAtCursor(MessageComponentType::Text);
        return;
    case RichFormat::Code:
        insertComponentAtCursor(MessageComponentType::Code);
        return;
    case RichFormat::Quote:
        insertComponentAtCursor(MessageComponentType::Quote);
        return;
    default:
        return;
    }
}

void ChatBarMessageContentModel::insertComponentAtCursor(MessageComponentType::Type type)
{
    if (m_components[m_currentFocusComponent.row()].type == type) {
        if (type == MessageComponentType::Text && focusedTextItem()) {
            focusedTextItem()->mergeFormatOnCursor(RichFormat::Paragraph);
        }
        return;
    }

    bool hasBefore = false;
    QTextDocumentFragment midFragment;
    std::optional<QTextDocumentFragment> afterFragment = std::nullopt;

    if (const auto currentTextItem = focusedTextItem()) {
        currentTextItem->fillFragments(hasBefore, midFragment, afterFragment);
    }

    const auto currentType = m_components[m_currentFocusComponent.row()].type;
    int insertRow = m_currentFocusComponent.row() + (hasBefore ? 1 : 0);

    if (!hasBefore) {
        removeComponent(insertRow, true);
    }

    const auto insertIt = insertComponent(insertRow, type);
    if (insertIt != m_components.end()) {
        if (const auto insertTextItem = textItemForComponent(*insertIt)) {
            insertTextItem->insertFragment(midFragment);
        }
        m_currentFocusComponent = QPersistentModelIndex(index(insertIt - m_components.begin()));
        emitFocusChangeSignals();
    }

    if (afterFragment) {
        const auto afterIt = insertComponent(insertRow + 1, currentType);
        if (afterIt != m_components.end()) {
            if (const auto afterTextItem = textItemForComponent(*afterIt)) {
                afterTextItem->insertFragment(*afterFragment);
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

    if (const auto textItem = textItemForIndex(index(row))) {
        m_components[row].attributes.remove(TextItemKey);
        textItem->disconnect(this);
        textItem->deleteLater();
    }
    it = m_components.erase(it);
    endRemoveRows();

    Q_EMIT hasRichFormattingChanged();

    return it;
}

void ChatBarMessageContentModel::removeComponent(ChatTextItemHelper *textItem)
{
    const auto index = indexForTextItem(textItem);
    if (index.isValid()) {
        removeComponent(index.row());
    }
}

void ChatBarMessageContentModel::handleBlockTransition(bool up)
{
    const auto currentRow = m_currentFocusComponent.row();
    const auto insertRow = currentRow + (up ? 0 : 1);
    const auto atEdge = up ? currentRow <= 0 : currentRow >= rowCount() - 1;
    const auto notText = focusType() != MessageComponentType::Text;
    if (atEdge && notText) {
        insertComponent(insertRow, MessageComponentType::Text);
        setFocusRow(insertRow);
        return;
    }

    const auto nextRow = currentRow + (up ? -1 : 1);
    const auto nextNotText = m_components[nextRow].type != MessageComponentType::Text;
    if (notText && nextNotText) {
        insertComponent(insertRow, MessageComponentType::Text);
        setFocusRow(insertRow);
        return;
    }

    const auto currentItemEmptyText = focusedTextItem()->isEmpty() && focusType() == MessageComponentType::Text;
    setFocusRow(nextRow);
    if (currentItemEmptyText && !atEdge) {
        removeComponent(currentRow);
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
            stringOut += u"\n"_s;
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
            if (const auto textItem = textItemForComponent(component)) {
                auto newText = textItem->markdownText();
                newText.replace(QRegularExpression(u"(?<!\n)\n(?!\n)"_s), u" "_s);
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
    clearModel();
    initializeModel();
}

std::optional<QString> ChatBarMessageContentModel::getReplyEventId()
{
    if (!m_room) {
        return std::nullopt;
    }
    return m_room->mainCache()->isReplying() ? std::make_optional(m_room->mainCache()->replyId()) : std::nullopt;
}

void ChatBarMessageContentModel::clearModel()
{
    beginResetModel();
    for (const auto &component : m_components) {
        if (const auto textItem = textItemForComponent(component)) {
            textItem->disconnect(this);
            textItem->deleteLater();
        }
    }
    m_components.clear();
    endResetModel();
}

#include "moc_chatbarmessagecontentmodel.cpp"
