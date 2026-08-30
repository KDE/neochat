// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatbarmessagecontentmodel.h"

#include <QMimeData>
#include <QMimeDatabase>
#include <QMovie>
#include <QTextDocumentFragment>
#include <QTimer>

#include <KLocalizedString>
#include <KUrlMimeData>
#include <Kirigami/Platform/PlatformTheme>

#include "block.h"
#include "blockcache.h"
#include "blockreply.h"
#include "chatkeyhelper.h"
#include "chatmarkdownhelper.h"
#include "chattextitemhelper.h"
#include "enums/blocktype.h"
#include "enums/richformat.h"
#include "fileinfo.h"
#include "texthandler.h"

bool ChatBarMessageContentModel::richTextActive = true;

ChatBarMessageContentModel::ChatBarMessageContentModel(QObject *parent)
    : MessageContentModel(parent)
    , m_markdownHelper(new ChatMarkdownHelper(this))
    , m_keyHelper(new ChatKeyHelper(this))
{
    m_editableActive = true;

    connect(this, &ChatBarMessageContentModel::focusRowChanged, this, [this]() {
        m_markdownHelper->setTextItem(focusedTextItem());
        m_keyHelper->setTextItem(focusedTextItem());
    });
    connect(m_markdownHelper, &ChatMarkdownHelper::unhandledBlockFormat, this, &ChatBarMessageContentModel::insertStyleAtCursor);
    connect(this, &ChatBarMessageContentModel::modelReset, this, &ChatBarMessageContentModel::contentChanged);
    connect(this, &ChatBarMessageContentModel::rowsInserted, this, &ChatBarMessageContentModel::contentChanged);
    connect(this, &ChatBarMessageContentModel::rowsRemoved, this, &ChatBarMessageContentModel::contentChanged);

    connectKeyHelper();
    initializeModel();
}

void ChatBarMessageContentModel::initializeModel(const QString &initialText)
{
    beginInsertRows({}, rowCount(), rowCount());
    auto textBlock = new Blocks::TextBlock(Blocks::Text, QTextDocumentFragment::fromPlainText(initialText), false, this);
    connectTextItem(textBlock->item());
    m_components.push_back(std::move(textBlock));
    endInsertRows();

    m_currentFocusComponent = QPersistentModelIndex(index(0));
    Q_EMIT focusRowChanged();
}

void ChatBarMessageContentModel::initializeFromCache()
{
    if (!m_cache) {
        return;
    }

    clearModel();
    if (m_cache->empty()) {
        initializeModel();
        return;
    }

    beginResetModel();
    std::ranges::for_each(m_cache->cbegin(), m_cache->cend(), [this](std::unique_ptr<Blocks::CacheItem> const &cacheItem) {
        insertComponentFromCache(cacheItem.get());
    });
    endResetModel();

    m_currentFocusComponent = QPersistentModelIndex(index(rowCount() - 1));
    Q_EMIT focusRowChanged();
}

Blocks::Cache *ChatBarMessageContentModel::cache() const
{
    return m_cache;
}

void ChatBarMessageContentModel::setCache(Blocks::Cache *cache)
{
    if (cache == m_cache) {
        return;
    }
    m_cache = cache;

    if (m_cache) {
        initializeFromCache();
        refocusCurrentComponent();
    }
    Q_EMIT cacheChanged();
}

ChatKeyHelper *ChatBarMessageContentModel::keyHelper() const
{
    return m_keyHelper;
}

void ChatBarMessageContentModel::connectKeyHelper()
{
    connect(m_keyHelper, &ChatKeyHelper::unhandledUp, this, [this](bool isCompleting, Qt::KeyboardModifiers modifiers) {
        if (isCompleting) {
            return;
        }
        if (m_currentFocusComponent.row() <= 0) {
            Q_EMIT unhandledUp(modifiers);
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
            if (const auto nextTextItem = textItemForComponent(m_components[currentRow + 1])) {
                focusedTextItem()->insertFragment(nextTextItem->takeFirstBlock(), ChatTextItemHelper::Cursor, true);
            }
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::unhandledBackspace, this, [this]() {
        const auto currentRow = m_currentFocusComponent.row();
        if (currentRow > 0) {
            const auto previousRow = currentRow - 1;
            if (Blocks::isFileType(m_components[previousRow]->type())) {
                removeAttachment();
                return;
            }
            if (const auto previousTextItem = textItemForComponent(m_components.at(previousRow))) {
                previousTextItem->insertFragment(focusedTextItem()->takeFirstBlock(), ChatTextItemHelper::End, true);
                return;
            }
        }
        if (m_components[currentRow]->type() == Blocks::Code || m_components[currentRow]->type() == Blocks::Quote) {
            insertBlockAtCursor(makeEmptyTextBlock(Blocks::Text));
        }
    });
    connect(m_keyHelper, &ChatKeyHelper::attachmentPasted, this, &ChatBarMessageContentModel::addAttachment);
    connect(m_keyHelper, &ChatKeyHelper::textPasted, this, &ChatBarMessageContentModel::insertTextAtCursor);
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
    connect(chattextitemhelper, &ChatTextItemHelper::contentsChanged, this, &ChatBarMessageContentModel::contentChanged);
}

ChatTextItemHelper *ChatBarMessageContentModel::textItemForComponent(Blocks::Block *component) const
{
    if (const auto textBlock = dynamic_cast<Blocks::TextBlock *>(component)) {
        return textBlock->item();
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
    return std::ranges::any_of(m_components, [this](Blocks::Block *component) {
        if (component->type() != Blocks::Text && component->type() != Blocks::Reply) {
            return true;
        }
        if (const auto textItem = textItemForComponent(component)) {
            if (textItem->hasRichFormatting()) {
                return true;
            }
        }
        return false;
    });
}

void ChatBarMessageContentModel::insertTextAtCursor(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    const auto blocks = TextHandler().textComponents(text);
    std::ranges::for_each(blocks, [this](Blocks::Block *block) {
        insertBlockAtCursor(block);
    });
}

bool ChatBarMessageContentModel::hasAttachment() const
{
    return hasComponentType({Blocks::File, Blocks::Audio, Blocks::Image, Blocks::Video, Blocks::Location});
}

void ChatBarMessageContentModel::addAttachment(const QUrl &path)
{
    QString plainText;
    for (const auto &component : std::as_const(m_components)) {
        if (const auto textItem = textItemForComponent(component)) {
            plainText += u"%1%2"_s.arg(plainText.isEmpty() ? u""_s : u"\n"_s, textItem->plainText());
        }
    }

    clearModel(ClearModelOptions::KeepReply);
    initializeModel(plainText);

    auto it = insertComponent(m_components.front()->type() == Blocks::Reply ? 1 : 0, blockForFile(path));
    const auto componentIndex = index(std::distance(m_components.begin(), it));
    Q_EMIT dataChanged(componentIndex, componentIndex, {BlockRole});
    Q_EMIT hasAttachmentChanged();
}

void ChatBarMessageContentModel::addReply(MessageContentModel *blockModel, bool updateCache)
{
    if (!hasComponentType(Blocks::Reply)) {
        insertComponent(0, new Blocks::ReplyBlock(blockModel, this));
    } else {
        if (const auto replyBlock = dynamic_cast<Blocks::ReplyBlock *>(m_components[0])) {
            replyBlock->setBlockModel(blockModel);
        }
        dataChanged(index(0), index(0), {ReplyContentModelRole});
    }
    if (updateCache) {
        this->updateCache();
    }
    refocusCurrentComponent();
}

void ChatBarMessageContentModel::addLocation(qreal latitude, qreal longitude, const QString &asset)
{
    clearModel();
    auto assetString = asset == u"m.pin"_s ? i18n("pin") : i18n("location");
    auto authorString = u"User"_s;
    if (author()) {
        authorString = author()->displayName();
    }
    initializeModel(u"%1's %2"_s.arg(authorString, assetString));
    auto it =
        insertComponent(m_components.front()->type() == Blocks::Reply ? 1 : 0, new Blocks::LocationBlock(Blocks::Location, latitude, longitude, asset, this));
    const auto componentIndex = index(std::distance(m_components.begin(), it));
    Q_EMIT dataChanged(componentIndex, componentIndex, {BlockRole});
    Q_EMIT hasAttachmentChanged();
}

void ChatBarMessageContentModel::removeReply()
{
    if (m_components[0]->type() == Blocks::Reply) {
        removeComponent(0);
    }
    if (m_replyModel) {
        m_replyModel->disconnect(this);
        m_replyModel->deleteLater();
    }
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

Blocks::Block *ChatBarMessageContentModel::blockForFile(const QUrl &path)
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
        return new Blocks::ImageBlock(Blocks::Image, path, path.fileName(), imageInfo, QUrl(), Blocks::ImageInfo(), true, this);
    }
    if (mime.name().contains(u"video"_s)) {
        Blocks::VideoInfo videoInfo;
        videoInfo.mimeType = mime;
        videoInfo.size = fileInfo.size();
        return new Blocks::VideoBlock(Blocks::Video, path, path.fileName(), videoInfo, QUrl(), Blocks::ImageInfo(), nullptr, {}, this);
    }
    if (mime.name().contains(u"audio"_s)) {
        Blocks::AudioInfo audioInfo;
        audioInfo.mimeType = mime;
        audioInfo.size = fileInfo.size();
        return new Blocks::AudioBlock(Blocks::Audio, path, path.fileName(), audioInfo, nullptr, {}, this);
    }

    Blocks::FileInfo info;
    info.mimeType = mime;
    info.size = fileInfo.size();
    return new Blocks::FileBlock(Blocks::File, path, path.fileName(), info, nullptr, {}, this);
}

Blocks::BlockPtrsIt ChatBarMessageContentModel::insertComponent(int row, Blocks::Block *block)
{
    if (row < 0 || row > rowCount()) {
        return m_components.end();
    }

    if (const auto textBlock = dynamic_cast<Blocks::TextBlock *>(block)) {
        connectTextItem(textBlock->item());
    }

    beginInsertRows({}, row, row);
    const auto it = m_components.insert(m_components.begin() + row, block);
    endInsertRows();
    Q_EMIT hasRichFormattingChanged();

    return it;
}

void ChatBarMessageContentModel::insertStyleAtCursor(RichFormat::Format style)
{
    switch (style) {
    case RichFormat::Paragraph:
        insertBlockAtCursor(makeEmptyTextBlock(Blocks::Text));
        return;
    case RichFormat::Code:
        insertBlockAtCursor(makeEmptyTextBlock(Blocks::Code));
        return;
    case RichFormat::Quote:
        insertBlockAtCursor(makeEmptyTextBlock(Blocks::Quote));
        return;
    default:
        return;
    }
}

void ChatBarMessageContentModel::insertBlockAtCursor(Blocks::Block *block)
{
    if (!m_currentFocusComponent.isValid() || !block) {
        return;
    }

    if (block->type() == m_components[m_currentFocusComponent.row()]->type()) {
        if (!block->isEmpty()) {
            if (const auto textBlock = dynamic_cast<Blocks::TextBlock *>(block)) {
                focusedTextItem()->insertFragment(textBlock->item()->initialFragment());
            }
        } else if ((block->type() == Blocks::Text || block->type() == Blocks::Quote)) {
            focusedTextItem()->mergeFormatOnCursor(RichFormat::Paragraph);
        }
        return;
    }

    bool hasBefore = false;
    std::optional<QTextDocumentFragment> midFragment = std::nullopt;
    std::optional<QTextDocumentFragment> afterFragment = std::nullopt;

    const auto currentTextItem = focusedTextItem();
    if (!currentTextItem) {
        return;
    }
    if (block->isEmpty()) {
        currentTextItem->fill3Fragments(hasBefore, midFragment, afterFragment);
    } else {
        currentTextItem->fill2Fragments(hasBefore, afterFragment);
    }

    const auto currentType = m_components[m_currentFocusComponent.row()]->type();
    int insertRow = m_currentFocusComponent.row() + (hasBefore ? 1 : 0);

    if (!hasBefore) {
        removeComponent(insertRow, true);
    }

    const auto insertIt = insertComponent(insertRow, block);
    if (insertIt != m_components.end()) {
        const auto insertTextItem = textItemForComponent(*insertIt);
        if (midFragment && insertTextItem) {
            insertTextItem->insertFragment(*midFragment);
        }
        m_currentFocusComponent = QPersistentModelIndex(index(insertIt - m_components.begin()));
        emitFocusChangeSignals();
    }

    if (afterFragment) {
        const auto afterIt = insertComponent(insertRow + 1, makeEmptyTextBlock(currentType));
        if (afterIt != m_components.end()) {
            if (const auto afterTextItem = textItemForComponent(*afterIt)) {
                afterTextItem->insertFragment(*afterFragment);
            }
        }
    }
}

Blocks::Block *ChatBarMessageContentModel::makeEmptyTextBlock(Blocks::Type type)
{
    switch (type) {
    case Blocks::Text:
    case Blocks::Quote: {
        auto block = new Blocks::TextBlock(type, QTextDocumentFragment(), false, this);
        connectTextItem(block->item());
        return block;
    }
    case Blocks::Code: {
        auto block = new Blocks::CodeBlock(type, QTextDocumentFragment(), QString(), this);
        connectTextItem(block->item());
        return block;
    }
    default:
        return new Blocks::Block(Blocks::Other, this);
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
    if (!hasComponentType({Blocks::File, Blocks::Audio, Blocks::Image, Blocks::Video, Blocks::Location})) {
        return;
    }

    auto mediaRow = 0;
    if (Blocks::isFileType(m_components[1]->type())) {
        mediaRow = 1;
    }
    const auto attachmentType = m_components[mediaRow]->type();
    removeComponent(mediaRow);
    if (attachmentType == Blocks::Location) {
        clearModel();
        initializeModel();
    }
    refocusCurrentComponent();
    Q_EMIT hasAttachmentChanged();
}

void ChatBarMessageContentModel::setImageOptimization(const bool optimize) const
{
    if (!hasComponentType(Blocks::Image)) {
        return;
    }

    auto mediaRow = 0;
    if (Blocks::isFileType(m_components[1]->type())) {
        mediaRow = 1;
    }

    dynamic_cast<Blocks::ImageBlock *>(m_components[mediaRow])->setOptimize(optimize);
    updateCache(); // so the optimization flag is actually reflected
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
    switch (item->type) {
    case Blocks::Text:
    case Blocks::Quote:
        insertComponent(rowCount(), new Blocks::TextBlock(dynamic_cast<Blocks::TextCacheItem *>(item), this));
        break;
    case Blocks::Code:
        insertComponent(rowCount(), new Blocks::CodeBlock(dynamic_cast<Blocks::CodeCacheItem *>(item), this));
        break;
    case Blocks::File:
        insertComponent(rowCount(), new Blocks::FileBlock(dynamic_cast<Blocks::FileCacheItem *>(item), this));
        break;
    case Blocks::Image:
        insertComponent(rowCount(), new Blocks::ImageBlock(dynamic_cast<Blocks::ImageCacheItem *>(item), this));
        break;
    case Blocks::Video:
        insertComponent(rowCount(), new Blocks::VideoBlock(dynamic_cast<Blocks::VideoCacheItem *>(item), this));
        break;
    case Blocks::Audio:
        insertComponent(rowCount(), new Blocks::AudioBlock(dynamic_cast<Blocks::AudioCacheItem *>(item), this));
        break;
    case Blocks::Location:
        insertComponent(rowCount(), new Blocks::LocationBlock(dynamic_cast<Blocks::LocationCacheItem *>(item), this));
        break;
    case Blocks::Reply:
        if (const auto replyItem = dynamic_cast<Blocks::ReplyCacheItem *>(item)) {
            addReply(replyItem->blockModel, false);
        }
        break;
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
    const auto nextNotText = m_components[nextRow]->type() != Blocks::Text;
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
    if (!m_cache) {
        return;
    }

    m_cache->clear();
    std::ranges::for_each(m_components, [this](Blocks::Block *component) {
        m_cache->append(component->toCacheItem());
    });
}

void ChatBarMessageContentModel::resetModel()
{
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
    if (const auto textItem = textItemForComponent(m_components.front())) {
        if (textItem->isEmpty()) {
            return false;
        }
    }

    return true;
}

void ChatBarMessageContentModel::clearModel(ClearModelOptions options)
{
    const auto hadAttachment = hasComponentType({Blocks::File, Blocks::Audio, Blocks::Image, Blocks::Video});

    beginResetModel();
    for (const auto &component : m_components) {
        if (const auto textItem = textItemForComponent(component)) {
            textItem->disconnect(this);
            textItem->deleteLater();
        }
    }
    std::erase_if(m_components, [options](const auto &component) {
        return component->type() != Blocks::Reply || options == ClearModelOptions::RemoveReply;
    });
    endResetModel();

    if (hadAttachment) {
        Q_EMIT hasAttachmentChanged();
    }
}

#include "moc_chatbarmessagecontentmodel.cpp"
