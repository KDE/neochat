// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "block.h"
#include "blockcache.h"
#include "fileinfo.h"
#include <memory>

using namespace Blocks;

Block::Block(QObject *parent)
    : QObject(parent)
{
}

Block::Block(Type type, QObject *parent)
    : QObject(parent)
    , type(type)
{
}

Block::Block(CacheItem *item, QObject *parent)
    : QObject(parent)
    , type(item->type)
{
}

CacheItemPtr Block::toCacheItem() const
{
    return std::make_unique<CacheItem>(type);
}

QVariant Block::toVariant() const
{
    return QVariant::fromValue(this);
}

TextBlock::TextBlock(QObject *parent)
    : Block(parent)
{
}

TextBlock::TextBlock(Type type, const QTextDocumentFragment &content, bool hasSpoiler, QObject *parent)
    : Block(type, parent)
    , hasSpoiler(hasSpoiler)
    , m_item(new ChatTextItemHelper(this))
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
    if (type == Blocks::Quote) {
        m_item->setFixedChars(u"“"_s, u"”"_s);
    }
}

ChatTextItemHelper *TextBlock::item() const
{
    return m_item;
}

CacheItemPtr TextBlock::toCacheItem() const
{
    return std::make_unique<TextCacheItem>(type, m_item->toFragment(), hasSpoiler);
}

QVariant TextBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

CodeBlock::CodeBlock(QObject *parent)
    : TextBlock(parent)
{
}

CodeBlock::CodeBlock(Type type, const QTextDocumentFragment &content, const QString &language, QObject *parent)
    : TextBlock(type, content, parent)
    , language(language)
{
}

CodeBlock::CodeBlock(CodeCacheItem *item, QObject *parent)
    : TextBlock(item, parent)
    , language(item->language)
{
}

CacheItemPtr CodeBlock::toCacheItem() const
{
    return std::make_unique<CodeCacheItem>(type, m_item->toFragment(), language);
}

QVariant CodeBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

UrlBlock::UrlBlock(QObject *parent)
    : Block(parent)
{
}

UrlBlock::UrlBlock(Type type, const QUrl &source, QObject *parent)
    : Block(type, parent)
    , source(source)
{
}

UrlBlock::UrlBlock(UrlCacheItem *item, QObject *parent)
    : Block(item, parent)
    , source(item->source)
{
}

CacheItemPtr UrlBlock::toCacheItem() const
{
    return std::make_unique<UrlCacheItem>(type, source);
}

QVariant UrlBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

FileBlock::FileBlock(QObject *parent)
    : UrlBlock(parent)
{
}

FileBlock::FileBlock(Type type, const QUrl &source, const QString &filename, const FileInfo &info, QObject *parent)
    : UrlBlock(type, source, parent)
    , filename(filename)
    , info(info)
{
}

FileBlock::FileBlock(FileCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , filename(item->filename)
    , info(item->info)
{
}

CacheItemPtr FileBlock::toCacheItem() const
{
    return std::make_unique<FileCacheItem>(type, source, filename, info);
}

QVariant FileBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

ImageBlock::ImageBlock(QObject *parent)
    : UrlBlock(parent)
{
}

ImageBlock::ImageBlock(Type type,
                       const QUrl &source,
                       const QString &filename,
                       const ImageInfo &info,
                       const QUrl &thumbnailSource,
                       const ImageInfo &thumbnailInfo,
                       QObject *parent)
    : UrlBlock(type, source, parent)
    , filename(filename)
    , info(info)
    , thumbnailSource(thumbnailSource)
    , thumbnailInfo(thumbnailInfo)
{
}

ImageBlock::ImageBlock(ImageCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , filename(item->filename)
    , info(item->info)
    , thumbnailSource(item->thumbnailSource)
    , thumbnailInfo(item->thumbnailInfo)
{
}

CacheItemPtr ImageBlock::toCacheItem() const
{
    return std::make_unique<ImageCacheItem>(type, source, filename, info);
}

QVariant ImageBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

VideoBlock::VideoBlock(QObject *parent)
    : UrlBlock(parent)
{
}

VideoBlock::VideoBlock(Type type,
                       const QUrl &source,
                       const QString &filename,
                       const VideoInfo &info,
                       const QUrl &thumbnailSource,
                       const ImageInfo &thumbnailInfo,
                       QObject *parent)
    : UrlBlock(type, source, parent)
    , filename(filename)
    , info(info)
    , thumbnailSource(thumbnailSource)
    , thumbnailInfo(thumbnailInfo)
{
}

VideoBlock::VideoBlock(VideoCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , filename(item->filename)
    , info(item->info)
    , thumbnailSource(item->thumbnailSource)
    , thumbnailInfo(item->thumbnailInfo)
{
}

CacheItemPtr VideoBlock::toCacheItem() const
{
    return std::make_unique<VideoCacheItem>(type, source, filename, info);
}

QVariant VideoBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

AudioBlock::AudioBlock(QObject *parent)
    : UrlBlock(parent)
{
}

AudioBlock::AudioBlock(Type type, const QUrl &source, const QString &filename, const AudioInfo &info, QObject *parent)
    : UrlBlock(type, source, parent)
    , filename(filename)
    , info(info)
{
}

AudioBlock::AudioBlock(AudioCacheItem *item, QObject *parent)
    : UrlBlock(item, parent)
    , filename(item->filename)
    , info(item->info)
{
}

CacheItemPtr AudioBlock::toCacheItem() const
{
    return std::make_unique<AudioCacheItem>(type, source, filename, info);
}

QVariant AudioBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

LocationBlock::LocationBlock(QObject *parent)
    : Block(parent)
{
}

LocationBlock::LocationBlock(Type type, qreal latitude, qreal longitude, const QString &asset, QObject *parent)
    : Block(type, parent)
    , latitude(latitude)
    , longitude(longitude)
    , asset(asset)
{
}

LocationBlock::LocationBlock(LocationCacheItem *item, QObject *parent)
    : Block(item, parent)
    , latitude(item->latitude)
    , longitude(item->longitude)
    , asset(item->asset)
{
}

CacheItemPtr LocationBlock::toCacheItem() const
{
    return std::make_unique<LocationCacheItem>(type, latitude, longitude, asset);
}

QVariant LocationBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

ReplyBlock::ReplyBlock(QObject *parent)
    : Block(parent)
{
}

ReplyBlock::ReplyBlock(Type type, const QString &id, QObject *parent)
    : Block(type, parent)
    , id(id)
{
}

ReplyBlock::ReplyBlock(ReplyCacheItem *item, QObject *parent)
    : Block(item, parent)
    , id(item->id)
{
}

CacheItemPtr ReplyBlock::toCacheItem() const
{
    return std::make_unique<ReplyCacheItem>(type, id);
}

QVariant ReplyBlock::toVariant() const
{
    return QVariant::fromValue(this);
}

#include "moc_block.cpp"
