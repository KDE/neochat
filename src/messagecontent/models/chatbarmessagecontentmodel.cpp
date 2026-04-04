// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include <QMimeData>
#include <QMovie>
#include <QTextDocumentFragment>
#include <QTimer>

#include <KUrlMimeData>
#include <Kirigami/Platform/PlatformTheme>

#include "block.h"
#include "blockcache.h"
#include "chatbarcache.h"
#include "chatkeyhelper.h"
#include "chatmarkdownhelper.h"
#include "chattextitemhelper.h"
#include "enums/blocktype.h"
#include "enums/chatbartype.h"
#include "enums/richformat.h"
#include "fileinfo.h"
#include "messagecontentmodel.h"
#include "neochatroom.h"

namespace
{
constexpr auto TextItemKey = "chatTextItemHelper"_L1;
}

bool ChatBarMessageContentModel::richTextActive = true;

ChatBarMessageContentModel::ChatBarMessageContentModel(QObject *parent)
    : MessageContentModel(parent)
    , m_markdownHelper(new ChatMarkdownHelper(this))
    , m_keyHelper(new ChatKeyHelper(this))
    , m_typingTimer(new QTimer(this))
{
    m_editableActive = true;

    m_typingTimer->setInterval(std::chrono::milliseconds(5000));
    m_typingTimer->setSingleShot(true);

    connect(this, &ChatBarMessageContentModel::roomChanged, this, [this](NeoChatRoom *oldRoom) {
        if (m_type == ChatBarType::None || !m_room) {
            return;
        }

        connectCache(oldRoom ? oldRoom->cacheForType(m_type) : nullptr);
        initializeFromCache();
        refocusCurrentComponent();
    });
    connect(this, &ChatBarMessageContentModel::focusRowChanged, this, [this]() {
        m_markdownHelper->setTextItem(focusedTextItem());
        m_keyHelper->setTextItem(focusedTextItem());
    });
    connect(this, &ChatBarMessageContentModel::roomChanged, this, [this]() {
        m_keyHelper->room = m_room;
        // We can't guarantee whether room or type is intialised first so we have to handle.
        if (!m_room || !unhandledTypeChange) {
            return;
        }
        connectCache(m_room->cacheForType(*unhandledTypeChange));
        unhandledTypeChange = std::nullopt;
        const auto newCache = m_room->cacheForType(m_type);
        if (newCache && newCache->isEditing()) {
            initializeEdit();
            return;
        }
        initializeFromCache();
    });
    connect(this, &ChatBarMessageContentModel::typeChanged, this, [this](ChatBarType::Type oldType) {
        if (!m_room) {
            unhandledTypeChange = oldType;
            return;
        }
        connectCache(m_room->cacheForType(oldType));
        const auto newCache = m_room->cacheForType(m_type);
        if (newCache && newCache->isEditing()) {
            initializeEdit();
            return;
        }
        initializeFromCache();
    });
    connect(m_markdownHelper, &ChatMarkdownHelper::unhandledBlockFormat, this, &ChatBarMessageContentModel::insertStyleAtCursor);
    connect(this, &ChatBarMessageContentModel::modelReset, this, &ChatBarMessageContentModel::hasAnyContentChanged);
    connect(this, &ChatBarMessageContentModel::rowsInserted, this, &ChatBarMessageContentModel::hasAnyContentChanged);
    connect(this, &ChatBarMessageContentModel::rowsRemoved, this, &ChatBarMessageContentModel::hasAnyContentChanged);

    connectCache();
    connectKeyHelper();
    initializeModel();
}

void ChatBarMessageContentModel::connectCache(ChatBarCache *oldCache)
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    if (oldCache) {
        oldCache->disconnect(this);
    }

    connect(m_room->cacheForType(m_type), &ChatBarCache::relationIdChanged, this, [this]() {
        if (!m_room || m_type == ChatBarType::None) {
            return;
        }
        const auto currentCache = m_room->cacheForType(m_type);
        updateReplyModel();
        refocusCurrentComponent();
        if (currentCache->isEditing()) {
            initializeEdit();
        }
    });
}

void ChatBarMessageContentModel::initializeModel(const QString &initialText)
{
    updateReplyModel();

    beginInsertRows({}, rowCount(), rowCount());
    auto textBlock = std::make_unique<Blocks::TextBlock>(Blocks::Text, QTextDocumentFragment::fromPlainText(initialText));
    connectTextItem(textBlock->item());
    m_components.push_back(std::move(textBlock));
    endInsertRows();

    m_currentFocusComponent = QPersistentModelIndex(index(0));
    Q_EMIT focusRowChanged();
}

void ChatBarMessageContentModel::initializeFromCache()
{
    clearModel();

    const auto &currentCache = m_room->cacheForType(m_type);
    const auto &blockCache = currentCache->cache();
    if (blockCache.empty()) {
        initializeModel();
        return;
    }

    beginResetModel();
    std::ranges::for_each(blockCache.cbegin(), blockCache.cend(), [this](std::unique_ptr<Blocks::CacheItem> const &cacheItem) {
        insertComponentFromCache(cacheItem.get());
    });
    endResetModel();

    m_currentFocusComponent = QPersistentModelIndex(index(rowCount() - 1));
    Q_EMIT focusRowChanged();
}

inline QString trimmedTrailing(QString string)
{
    while (string.endsWith(u' ')) {
        string.removeLast();
    }
    return string;
}

inline QString trimNewline(QString string)
{
    while (string.startsWith(u"\n"_s)) {
        string.removeFirst();
    }
    while (string.endsWith(u"\n"_s)) {
        string.removeLast();
    }
    return string;
}

void ChatBarMessageContentModel::initializeEdit()
{
    clearModel();

    const auto currentCache = m_room->cacheForType(m_type);
    if (!richTextActive) {
        auto doc = QTextDocument();
        doc.setHtml(currentCache->relationMessage().replace(u'\n', u""_s));
        auto cursor = QTextCursor(&doc);
        QString escapedText;
        while (!cursor.atEnd()) {
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            auto nextText = trimmedTrailing(trimNewline(cursor.selection().toMarkdown()));
            if (!cursor.currentList()) {
                nextText.replace(u'\n', u' ');
                nextText.replace(u"  "_s, u" "_s);
            }
            if (!escapedText.isEmpty() && !nextText.isEmpty()) {
                escapedText += cursor.currentList() ? u"\n"_s : u"\n\n"_s;
            }
            escapedText += nextText;
            cursor.movePosition(QTextCursor::NextBlock);
        }
        doc.setPlainText(escapedText);
        QRegularExpression mentionRegex(u"\\[(.*?)]\\((.*?)\\)"_s);
        const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
        auto nextMention = doc.find(mentionRegex, 0);
        while (nextMention.hasSelection()) {
            const auto mentionMatch = mentionRegex.match(nextMention.selectedText());
            auto mentionName = mentionMatch.captured(1);
            if (mentionName.startsWith(u"\\"_s)) {
                mentionName.remove(0, 1);
            }
            nextMention.removeSelectedText();
            QTextCharFormat mentionFormat;
            mentionFormat.setForeground(theme->linkColor());
            mentionFormat.setFontWeight(QFont::Bold);
            mentionFormat.setAnchor(true);
            mentionFormat.setAnchorHref(mentionMatch.captured(2));
            nextMention.insertText(mentionName, mentionFormat);
            nextMention = doc.find(mentionRegex, nextMention.position());
        }
        cursor.movePosition(QTextCursor::End);
        cursor.select(QTextCursor::Document);
        auto textBlock = std::make_unique<Blocks::TextBlock>(Blocks::Text, cursor.selection());
        connectTextItem(textBlock->item());
        insertComponent(0, std::move(textBlock));
        return;
    }

    auto components = currentCache->relationComponents();
    if (components.empty()) {
        initializeModel();
        return;
    }
    m_components.insert(m_components.end(), std::make_move_iterator(components.begin()), std::make_move_iterator(components.end()));

    beginResetModel();
    std::ranges::for_each(m_components, [this](const Blocks::BlockPtr &component) {
        if (const auto textBlock = dynamic_cast<Blocks::TextBlock *>(component.get())) {
            connectTextItem(textBlock->item());
            return;
        }
        if (const auto codeBlock = dynamic_cast<Blocks::CodeBlock *>(component.get())) {
            connectTextItem(codeBlock->item());
            return;
        }
    });
    endResetModel();
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
    const auto oldType = std::exchange(m_type, type);
    Q_EMIT typeChanged(oldType, m_type);
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
        if (currentRow < (int)m_components.size() - 1) {
            if (const auto nextTextItem = textItemForComponent(m_components[currentRow].get())) {
                focusedTextItem()->insertFragment(nextTextItem->takeFirstBlock(), ChatTextItemHelper::Cursor, true);
            }
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::unhandledBackspace, this, [this]() {
        const auto currentRow = m_currentFocusComponent.row();
        if (currentRow > 0) {
            const auto previousRow = currentRow - 1;
            if (Blocks::isFileType(m_components[previousRow]->type)) {
                removeAttachment();
                return;
            }
            if (const auto previousTextItem = textItemForComponent(m_components.at(previousRow).get())) {
                previousTextItem->insertFragment(focusedTextItem()->takeFirstBlock(), ChatTextItemHelper::End, true);
                return;
            }
        }
        if (m_components[currentRow]->type == Blocks::Code || m_components[currentRow]->type == Blocks::Quote) {
            insertBlockAtCursor(Blocks::Text);
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::unhandledReturn, this, [this](bool isCompleting) {
        if (!isCompleting) {
            postMessage();
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::imagePasted, this, [this](const QString &filePath) {
        addAttachment(QUrl(filePath));
    });
}

int ChatBarMessageContentModel::focusRow() const
{
    return m_currentFocusComponent.row();
}

Blocks::Type ChatBarMessageContentModel::focusType() const
{
    return static_cast<Blocks::Type>(m_currentFocusComponent.data(ComponentTypeRole).toInt());
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
    connect(chattextitemhelper, &ChatTextItemHelper::contentsChanged, this, &ChatBarMessageContentModel::hasAnyContentChanged);
    connect(chattextitemhelper, &ChatTextItemHelper::contentsChanged, this, &ChatBarMessageContentModel::handleTyping);
}

ChatTextItemHelper *ChatBarMessageContentModel::textItemForComponent(Blocks::Block *component) const
{
    if (const auto textBlock = dynamic_cast<Blocks::TextBlock *>(component)) {
        return textBlock->item();
    }
    if (const auto codeBlock = dynamic_cast<Blocks::CodeBlock *>(component)) {
        return codeBlock->item();
    }
    return nullptr;
}

ChatTextItemHelper *ChatBarMessageContentModel::textItemForIndex(const QModelIndex &index) const
{
    return textItemForComponent(m_components[index.row()].get());
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
    return std::ranges::any_of(m_components, [this](const Blocks::BlockPtr &component) {
        if (component->type != Blocks::Text) {
            return true;
        }
        if (const auto textItem = textItemForComponent(component.get())) {
            if (textItem->hasRichFormatting()) {
                return true;
            }
        }
        return false;
    });
}

bool ChatBarMessageContentModel::hasAttachment() const
{
    return hasComponentType({Blocks::File, Blocks::Audio, Blocks::Image, Blocks::Video});
}

void ChatBarMessageContentModel::addAttachment(const QUrl &path)
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    QString plainText;
    for (const auto &component : std::as_const(m_components)) {
        if (const auto textItem = textItemForComponent(component.get())) {
            plainText += u"%1%2"_s.arg(plainText.isEmpty() ? u""_s : u"\n"_s, textItem->plainText());
        }
    }

    clearModel();
    initializeModel(plainText);

    auto it = insertComponent(m_components.front()->type == Blocks::Reply ? 1 : 0, blockForFile(path));
    Q_EMIT dataChanged(index(std::distance(m_components.begin(), it)), index(std::distance(m_components.begin(), it)), {BlockRole});
    Q_EMIT hasAttachmentChanged();
}

void ChatBarMessageContentModel::drop(QList<QUrl> u, const QString &transferPortal)
{
    QMimeData mimeData;
    mimeData.setUrls(u);
    if (!transferPortal.isEmpty()) {
        mimeData.setData(u"application/vnd.portal.filetransfer"_s, transferPortal.toLatin1());
    }
    auto urls = KUrlMimeData::urlsFromMimeData(&mimeData);
    if (urls.size() > 0) {
        addAttachment(urls[0]);
    }
}

Blocks::BlockPtr ChatBarMessageContentModel::blockForFile(const QUrl &path)
{
    QFileInfo fileInfo(path.isLocalFile() ? path.toLocalFile() : path.toString());
    auto mime = QMimeDatabase().mimeTypeForUrl(path);

    if (mime.name().contains(u"image"_s)) {
        Blocks::ImageInfo imageInfo;
        imageInfo.mimeType = mime;
        imageInfo.size = fileInfo.size();
        imageInfo.pixelSize = QImage(path.isLocalFile() ? path.toLocalFile() : path.toString()).size();

        // TODO: Images in certain formats (e.g. WebP) will be erroneously marked as animated, even if they are static.
        imageInfo.isAnimated = QMovie::supportedFormats().contains(mime.preferredSuffix().toUtf8());
        return Blocks::makeBlock<Blocks::ImageBlock>(Blocks::Image, path, path.fileName(), imageInfo, QUrl(), Blocks::ImageInfo());
    }
    if (mime.name().contains(u"video"_s)) {
        Blocks::VideoInfo videoInfo;
        videoInfo.mimeType = mime;
        videoInfo.size = fileInfo.size();
        return Blocks::makeBlock<Blocks::VideoBlock>(Blocks::Video, path, path.fileName(), videoInfo, QUrl(), Blocks::ImageInfo());
    }
    if (mime.name().contains(u"audio"_s)) {
        Blocks::AudioInfo audioInfo;
        audioInfo.mimeType = mime;
        audioInfo.size = fileInfo.size();
        return Blocks::makeBlock<Blocks::AudioBlock>(Blocks::Audio, path, path.fileName(), audioInfo);
    }
    return Blocks::makeBlock<Blocks::Block>(Blocks::Other);
}

Blocks::BlockPtrsIt ChatBarMessageContentModel::insertComponent(int row, Blocks::BlockPtr block)
{
    if (row < 0 || row > rowCount()) {
        return m_components.end();
    }

    beginInsertRows({}, row, row);
    const auto it = m_components.insert(m_components.begin() + row, std::move(block));
    endInsertRows();
    Q_EMIT hasRichFormattingChanged();

    return it;
}

void ChatBarMessageContentModel::insertStyleAtCursor(RichFormat::Format style)
{
    switch (style) {
    case RichFormat::Paragraph:
        insertBlockAtCursor(Blocks::Text);
        return;
    case RichFormat::Code:
        insertBlockAtCursor(Blocks::Code);
        return;
    case RichFormat::Quote:
        insertBlockAtCursor(Blocks::Quote);
        return;
    default:
        return;
    }
}

void ChatBarMessageContentModel::insertBlockAtCursor(Blocks::Type type)
{
    if (m_components[m_currentFocusComponent.row()]->type == type) {
        if ((type == Blocks::Text || type == Blocks::Quote) && focusedTextItem()) {
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

    const auto currentType = m_components[m_currentFocusComponent.row()]->type;
    int insertRow = m_currentFocusComponent.row() + (hasBefore ? 1 : 0);

    if (!hasBefore) {
        removeComponent(insertRow, true);
    }

    const auto insertIt = insertComponent(insertRow, makeEmptyTextBlock(type));
    if (insertIt != m_components.end()) {
        if (const auto insertTextItem = textItemForComponent(insertIt->get())) {
            insertTextItem->insertFragment(midFragment);
        }
        m_currentFocusComponent = QPersistentModelIndex(index(insertIt - m_components.begin()));
        emitFocusChangeSignals();
    }

    if (afterFragment) {
        const auto afterIt = insertComponent(insertRow + 1, makeEmptyTextBlock(currentType));
        if (afterIt != m_components.end()) {
            if (const auto afterTextItem = textItemForComponent(afterIt->get())) {
                afterTextItem->insertFragment(*afterFragment);
            }
        }
    }
}

Blocks::BlockPtr ChatBarMessageContentModel::makeEmptyTextBlock(Blocks::Type type)
{
    switch (type) {
    case Blocks::Text:
    case Blocks::Quote:
        return Blocks::makeBlock<Blocks::TextBlock>(type, QTextDocumentFragment());
    case Blocks::Code:
        return Blocks::makeBlock<Blocks::CodeBlock>(type, QTextDocumentFragment(), QString());
    default:
        return Blocks::makeBlock<Blocks::Block>(Blocks::Other);
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
    if (!hasComponentType({Blocks::File, Blocks::Audio, Blocks::Image, Blocks::Video})) {
        return;
    }

    auto mediaRow = 0;
    if (Blocks::isFileType(m_components[1]->type)) {
        mediaRow = 1;
    }
    removeComponent(mediaRow);
    refocusCurrentComponent();
    Q_EMIT hasAttachmentChanged();
}

bool ChatBarMessageContentModel::sendMessageWithEnter() const
{
    return m_sendMessageWithEnter;
}

void ChatBarMessageContentModel::setSendMessageWithEnter(bool sendMessageWithEnter)
{
    if (sendMessageWithEnter == m_sendMessageWithEnter) {
        return;
    }
    m_sendMessageWithEnter = sendMessageWithEnter;
    m_keyHelper->sendMessageWithEnter = sendMessageWithEnter;
    Q_EMIT sendMessageWithEnterChanged();
}

void ChatBarMessageContentModel::setSendTypingNotifications(bool sendTypingNotifications)
{
    m_sendTypingNotifications = sendTypingNotifications;
    if (!m_sendTypingNotifications && m_typingTimer->isActive()) {
        m_typingTimer->stop();
        m_room->sendTypingNotification(false);
    }
}

Blocks::BlockPtrsIt ChatBarMessageContentModel::removeComponent(Blocks::BlockPtrsIt it)
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
        textItem->disconnect(this);
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

void ChatBarMessageContentModel::insertComponentFromCache(Blocks::CacheItem *item)
{
    if (!Blocks::isTextType(item->type) && !Blocks::isFileType(item->type)) {
        return;
    }

    switch (item->type) {
    case Blocks::Text:
    case Blocks::Quote:
        insertComponent(rowCount(), Blocks::makeBlock<Blocks::TextBlock>(dynamic_cast<Blocks::TextCacheItem *>(item)));
        break;
    case Blocks::Code:
        insertComponent(rowCount(), Blocks::makeBlock<Blocks::CodeBlock>(dynamic_cast<Blocks::CodeCacheItem *>(item)));
        break;
    case Blocks::File:
        insertComponent(rowCount(), Blocks::makeBlock<Blocks::FileBlock>(dynamic_cast<Blocks::FileCacheItem *>(item)));
        break;
    case Blocks::Image:
        insertComponent(rowCount(), Blocks::makeBlock<Blocks::ImageBlock>(dynamic_cast<Blocks::ImageCacheItem *>(item)));
        break;
    case Blocks::Video:
        insertComponent(rowCount(), Blocks::makeBlock<Blocks::VideoBlock>(dynamic_cast<Blocks::VideoCacheItem *>(item)));
        break;
    case Blocks::Audio:
        insertComponent(rowCount(), Blocks::makeBlock<Blocks::AudioBlock>(dynamic_cast<Blocks::AudioCacheItem *>(item)));
    default:
        break;
    }
}

void ChatBarMessageContentModel::handleBlockTransition(bool up)
{
    const auto currentRow = m_currentFocusComponent.row();
    const auto insertRow = currentRow + (up ? 0 : 1);
    const auto atEdge = up ? currentRow <= 0 : currentRow >= rowCount() - 1;
    const auto notText = focusType() != Blocks::Text;
    if (atEdge && notText) {
        insertComponent(insertRow, makeEmptyTextBlock(Blocks::Text));
        setFocusRow(insertRow);
        return;
    }
    if (atEdge) {
        return;
    }

    const auto nextRow = currentRow + (up ? -1 : 1);
    const auto nextNotText = m_components[nextRow]->type != Blocks::Text;
    if (notText && nextNotText) {
        insertComponent(insertRow, makeEmptyTextBlock(Blocks::Text));
        setFocusRow(insertRow);
        return;
    }

    const auto currentItemEmptyText = focusedTextItem()->isEmpty() && focusType() == Blocks::Text;
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

    auto &cache = m_room->cacheForType(m_type)->cache();
    cache.clear();
    std::ranges::for_each(m_components, [&cache](const Blocks::BlockPtr &component) {
        cache.append(component->toCacheItem());
    });
}

void ChatBarMessageContentModel::resetModel()
{
    clearModel();
    initializeModel();
    refocusCurrentComponent();
}

void ChatBarMessageContentModel::postMessage()
{
    if (m_type == ChatBarType::None || !m_room) {
        return;
    }

    m_room->cacheForType(m_type)->postMessage();
    clearModel();
    initializeModel();
    refocusCurrentComponent();
}

bool ChatBarMessageContentModel::hasAnyContent() const
{
    // Shouldn't really be possible, but is true.
    if (m_components.empty()) {
        return false;
    }

    // If there's more than one component naturally there is content.
    if (m_components.size() > 1) {
        return true;
    }

    // There's usually at a minimum a TextComponent, we need to check if it's empty.
    if (const auto textItem = textItemForComponent(m_components.front().get())) {
        if (textItem->isEmpty()) {
            return false;
        }
    }

    return true;
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
    const auto hadAttachment = hasComponentType({Blocks::File, Blocks::Audio, Blocks::Image, Blocks::Video});

    beginResetModel();
    for (const auto &component : m_components) {
        if (const auto textItem = textItemForComponent(component.get())) {
            textItem->disconnect(this);
            textItem->deleteLater();
        }
    }
    m_components.clear();
    endResetModel();

    if (hadAttachment) {
        Q_EMIT hasAttachmentChanged();
    }
}

void ChatBarMessageContentModel::handleTyping()
{
    if (m_type == ChatBarType::None || !m_room || !m_sendTypingNotifications) {
        return;
    }

    if (!m_typingTimer->isActive() && hasAnyContent()) {
        m_typingTimer->start();
        m_room->sendTypingNotification(true);
    } else if (m_typingTimer->isActive() && !hasAnyContent()) {
        m_typingTimer->stop();
        m_room->sendTypingNotification(false);
    }
}

bool ChatBarMessageContentModel::sendTypingNotifications() const
{
    return m_sendTypingNotifications;
}

#include "moc_chatbarmessagecontentmodel.cpp"
