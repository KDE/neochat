// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "block.h"

#include <KLocalizedString>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "blockcache.h"
#include "blocktype.h"
#include "fileinfo.h"
#include "neochatroom.h"

using namespace Blocks;

Block::Block(Type type, QObject *parent)
    : QObject(parent)
    , m_type(type)
{
}

Block::Block(CacheItem *item, QObject *parent)
    : QObject(parent)
    , m_type(item->type)
{
}

Type Block::type() const
{
    return m_type;
}

void Block::setType(Type type)
{
    if (type == m_type) {
        return;
    }
    m_type = type;
    Q_EMIT typeChanged();
}

CacheItemPtr Block::toCacheItem() const
{
    return std::make_unique<CacheItem>(type());
}

QVariant Block::toVariant() const
{
    return QVariant::fromValue(this);
}

bool Block::operator==(const Block &right) const
{
    return type() == right.type();
}

bool Block::isEmpty() const
{
    return type() == Other;
}

BasicTextBlock::BasicTextBlock(Type type, const QString &display, QObject *parent)
    : Block(type, parent)
    , m_display(display)
{
}

BasicTextBlock::BasicTextBlock(BasicTextCacheItem *item, QObject *parent)
    : Block(item, parent)
    , m_display(item->display)
{
}

QString BasicTextBlock::display() const
{
    return m_display;
}

CacheItemPtr BasicTextBlock::toCacheItem() const
{
    return std::make_unique<BasicTextCacheItem>(type(), display());
}

TextBlock::TextBlock(Type type, const QTextDocumentFragment &content, bool hasSpoiler, QObject *parent)
    : Block(type, parent)
    , m_item(new ChatTextItemHelper(this))
    , m_hasSpoiler(hasSpoiler)
{
    m_item->setInitialFragment(content);
    if (type == Blocks::Quote) {
        m_item->setFixedChars(u"“"_s, u"”"_s);
    }
}

TextBlock::TextBlock(TextCacheItem *item, QObject *parent)
    : Block(item, parent)
    , m_item(new ChatTextItemHelper(this))
{
    m_item->setInitialFragment(item->content);
    if (type() == Blocks::Quote) {
        m_item->setFixedChars(u"“"_s, u"”"_s);
    }
}

ChatTextItemHelper *TextBlock::item() const
{
    return m_item;
}

bool TextBlock::hasSpoiler() const
{
    return m_hasSpoiler;
}

bool TextBlock::spoilerRevealed() const
{
    return m_spoilerRevealed;
}

void TextBlock::setSpoilerRevealed(bool spoilerRevealed)
{
    if (spoilerRevealed == m_spoilerRevealed) {
        return;
    }
    m_spoilerRevealed = spoilerRevealed;
    Q_EMIT spoilerRevealedChanged();
}

CacheItemPtr TextBlock::toCacheItem() const
{
    return std::make_unique<TextCacheItem>(type(), m_item->toFragment(), hasSpoiler());
}

CodeBlock::CodeBlock(Type type, const QTextDocumentFragment &content, const QString &language, QObject *parent)
    : TextBlock(type, content, {}, parent)
    , m_language(language)
{
}

CodeBlock::CodeBlock(CodeCacheItem *item, QObject *parent)
    : TextBlock(item, parent)
    , m_language(item->language)
{
}

QString CodeBlock::language() const
{
    return m_language;
}

CacheItemPtr CodeBlock::toCacheItem() const
{
    return std::make_unique<CodeCacheItem>(type(), item()->toFragment(), language());
}

UrlBlock::UrlBlock(Type type, const QUrl &source, QObject *parent)
    : Block(type, parent)
    , m_source(source)
{
}

UrlBlock::UrlBlock(UrlCacheItem *item, QObject *parent)
    : Block(item, parent)
    , m_source(item->source)
{
}

QUrl UrlBlock::source() const
{
    return m_source;
}

CacheItemPtr UrlBlock::toCacheItem() const
{
    return std::make_unique<UrlCacheItem>(type(), source());
}

FileBlock::FileBlock(Type type, const QUrl &source, const QString &filename, const FileInfo &info, NeoChatRoom *room, const QString &eventId, QObject *parent)
    : UrlBlock(type, source, parent)
    , m_filename(filename)
    , m_info(info)
    , m_room(room)
    , m_eventId(eventId)
{
    if (m_room) {
        connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId, const QString &errorMessage) {
            if (eventId != m_eventId || m_room == nullptr) {
                return;
            }
            Q_EMIT fileTransferInfoChanged();
            const auto message = errorMessage.isEmpty()
                ? i18nc("@info", "Failed to download file.")
                : i18nc("@info Failed to download file: [error message]", "Failed to download file:<br />%1", errorMessage);
            Q_EMIT m_room->showMessage(MessageType::Error, message);
        });
    }
}

FileBlock::FileBlock(FileCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , m_filename(item->filename)
    , m_info(item->info)
{
}

QString FileBlock::filename() const
{
    return m_filename;
}

const FileInfo &FileBlock::info() const
{
    return m_info;
}

Quotient::FileTransferInfo FileBlock::fileTransferInfo() const
{
    if (!m_room) {
        return {};
    }
    return m_room->cachedFileTransferInfo(m_eventId);
}

CacheItemPtr FileBlock::toCacheItem() const
{
    return std::make_unique<FileCacheItem>(type(), source(), filename(), info());
}

ImageBlock::ImageBlock(Type type,
                       const QUrl &source,
                       const QString &filename,
                       const ImageInfo &info,
                       const QUrl &thumbnailSource,
                       const ImageInfo &thumbnailInfo,
                       QObject *parent)
    : UrlBlock(type, source, parent)
    , m_filename(filename)
    , m_info(info)
    , m_thumbnailSource(thumbnailSource)
    , m_thumbnailInfo(thumbnailInfo)
{
}

ImageBlock::ImageBlock(ImageCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , m_filename(item->filename)
    , m_info(item->info)
    , m_thumbnailSource(item->thumbnailSource)
    , m_thumbnailInfo(item->thumbnailInfo)
{
}

QString ImageBlock::filename() const
{
    return m_filename;
}

const ImageInfo &ImageBlock::info() const
{
    return m_info;
}

QUrl ImageBlock::thumbnailSource() const
{
    return m_thumbnailSource;
}

const ImageInfo &ImageBlock::thumbnailInfo() const
{
    return m_thumbnailInfo;
}

CacheItemPtr ImageBlock::toCacheItem() const
{
    return std::make_unique<ImageCacheItem>(type(), source(), filename(), info(), thumbnailSource(), thumbnailInfo());
}

VideoBlock::VideoBlock(Type type,
                       const QUrl &source,
                       const QString &filename,
                       const VideoInfo &info,
                       const QUrl &thumbnailSource,
                       const ImageInfo &thumbnailInfo,
                       NeoChatRoom *room,
                       const QString &eventId,
                       QObject *parent)
    : UrlBlock(type, source, parent)
    , m_filename(filename)
    , m_info(info)
    , m_thumbnailSource(thumbnailSource)
    , m_thumbnailInfo(thumbnailInfo)
    , m_room(room)
    , m_eventId(eventId)
{
    if (m_room) {
        connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId, const QString &errorMessage) {
            if (eventId != m_eventId || m_room == nullptr) {
                return;
            }
            Q_EMIT fileTransferInfoChanged();
            const auto message = errorMessage.isEmpty()
                ? i18nc("@info", "Failed to download file.")
                : i18nc("@info Failed to download file: [error message]", "Failed to download file:<br />%1", errorMessage);
            Q_EMIT m_room->showMessage(MessageType::Error, message);
        });
    }
}

VideoBlock::VideoBlock(VideoCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , m_filename(item->filename)
    , m_info(item->info)
    , m_thumbnailSource(item->thumbnailSource)
    , m_thumbnailInfo(item->thumbnailInfo)
{
}

QString VideoBlock::filename() const
{
    return m_filename;
}

const VideoInfo &VideoBlock::info() const
{
    return m_info;
}

QUrl VideoBlock::thumbnailSource() const
{
    return m_thumbnailSource;
}

const ImageInfo &VideoBlock::thumbnailInfo() const
{
    return m_thumbnailInfo;
}

Quotient::FileTransferInfo VideoBlock::fileTransferInfo() const
{
    if (!m_room) {
        return {};
    }
    return m_room->cachedFileTransferInfo(m_eventId);
}

CacheItemPtr VideoBlock::toCacheItem() const
{
    return std::make_unique<VideoCacheItem>(type(), source(), filename(), info(), thumbnailSource(), thumbnailInfo());
}

AudioBlock::AudioBlock(Type type,
                       const QUrl &source,
                       const QString &filename,
                       const AudioInfo &info,
                       NeoChatRoom *room,
                       const QString &eventId,
                       QObject *parent)
    : UrlBlock(type, source, parent)
    , m_filename(filename)
    , m_info(info)
    , m_room(room)
    , m_eventId(eventId)
{
    if (m_room) {
        connect(m_room, &NeoChatRoom::newFileTransfer, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferProgress, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferCompleted, this, [this](const QString &eventId) {
            if (eventId == m_eventId) {
                Q_EMIT fileTransferInfoChanged();
            }
        });
        connect(m_room, &NeoChatRoom::fileTransferFailed, this, [this](const QString &eventId, const QString &errorMessage) {
            if (eventId != m_eventId || m_room == nullptr) {
                return;
            }
            Q_EMIT fileTransferInfoChanged();
            const auto message = errorMessage.isEmpty()
                ? i18nc("@info", "Failed to download file.")
                : i18nc("@info Failed to download file: [error message]", "Failed to download file:<br />%1", errorMessage);
            Q_EMIT m_room->showMessage(MessageType::Error, message);
        });
    }
}

AudioBlock::AudioBlock(AudioCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , m_filename(item->filename)
    , m_info(item->info)
{
}

QString AudioBlock::filename() const
{
    return m_filename;
}

const AudioInfo &AudioBlock::info() const
{
    return m_info;
}

Quotient::FileTransferInfo AudioBlock::fileTransferInfo() const
{
    if (!m_room) {
        return {};
    }
    return m_room->cachedFileTransferInfo(m_eventId);
}

CacheItemPtr AudioBlock::toCacheItem() const
{
    return std::make_unique<AudioCacheItem>(type(), source(), filename(), info());
}

LinkPreviewBlock::LinkPreviewBlock(Type type, const QUrl &source, NeoChatConnection *connection, QObject *parent)
    : UrlBlock(type, source, parent)
    , m_connection(connection)
{
}

LinkPreviewer *LinkPreviewBlock::linkPreviewer() const
{
    if (m_connection) {
        return m_connection->previewerForLink(source());
    } else {
        return emptyLinkPreview;
    }
}

ItineraryBlock::ItineraryBlock(Type type, const QUrl &source, QObject *parent)
    : Block(type, parent)
    , m_model(new ItineraryModel(source, this))
{
}

ItineraryModel *ItineraryBlock::model() const
{
    return m_model;
}

LocationBlock::LocationBlock(Type type, qreal latitude, qreal longitude, const QString &asset, QObject *parent)
    : Block(type, parent)
    , m_latitude(latitude)
    , m_longitude(longitude)
    , m_asset(asset)
{
}

LocationBlock::LocationBlock(LocationCacheItem *item, QObject *parent)
    : Block(item, parent)
    , m_latitude(item->latitude)
    , m_longitude(item->longitude)
    , m_asset(item->asset)
{
}

qreal LocationBlock::latitude() const
{
    return m_latitude;
}

qreal LocationBlock::longitude() const
{
    return m_longitude;
}

QString LocationBlock::asset() const
{
    return m_asset;
}

CacheItemPtr LocationBlock::toCacheItem() const
{
    return std::make_unique<LocationCacheItem>(type(), latitude(), longitude(), asset());
}

ReplyBlock::ReplyBlock(Type type, const QString &id, QObject *parent)
    : Block(type, parent)
    , m_id(id)
{
}

ReplyBlock::ReplyBlock(ReplyCacheItem *item, QObject *parent)
    : Block(item, parent)
    , m_id(item->id)
{
}

QString ReplyBlock::id() const
{
    return m_id;
}

void ReplyBlock::setId(const QString &id)
{
    if (id == m_id) {
        return;
    }
    m_id = id;
    Q_EMIT idChanged();
}

CacheItemPtr ReplyBlock::toCacheItem() const
{
    return std::make_unique<ReplyCacheItem>(type(), id());
}

ReactionBlock::ReactionBlock(Type type, NeoChatRoom *room, const QString &eventId, QObject *parent)
    : Block(type, parent)
    , m_model(new ReactionModel(this, eventId, room))
{
}

ReactionModel *ReactionBlock::model() const
{
    return m_model;
}

ChatBarBlock::ChatBarBlock(Type type, bool isEditing, const QString &threadRootId, QObject *parent)
    : Block(type, parent)
    , m_isEditing(isEditing)
    , m_threadRootId(threadRootId)
{
}

bool ChatBarBlock::isEditing() const
{
    return m_isEditing;
}

QString ChatBarBlock::threadRootId() const
{
    return m_threadRootId;
}

#include "moc_block.cpp"
