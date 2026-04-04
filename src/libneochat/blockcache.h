// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QList>
#include <QTextDocumentFragment>

#include "enums/blocktype.h"
#include "fileinfo.h"

namespace Blocks
{
/**
 * @class CacheItem
 *
 * A structure to define an item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class CacheItem
{
public:
    CacheItem(Type type);
    virtual ~CacheItem();

    Type type = Other;

    /**
     * @brief Return the contents of the CacheItem as a single string.
     */
    virtual QString toString() const;

    static bool richTextActive;
};

/**
 * @class TextCacheItem
 *
 * A structure to define a text item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class TextCacheItem : public CacheItem
{
public:
    TextCacheItem(Type type, const QTextDocumentFragment &content, bool hasSpoiler = {});

    QTextDocumentFragment content;
    bool hasSpoiler;

    QString toString() const override;
};

/**
 * @class TextCacheItem
 *
 * A structure to define a text item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class CodeCacheItem : public TextCacheItem
{
public:
    CodeCacheItem(Type type, const QTextDocumentFragment &content, const QString &language = {});

    QString language;

    QString toString() const override;
};

/**
 * @class UrlCacheItem
 *
 * A structure to define a url based item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class UrlCacheItem : public CacheItem
{
public:
    UrlCacheItem(Type type, const QUrl &source);

    QUrl source;
};

/**
 * @class FileCacheItem
 *
 * A structure to define a file item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class FileCacheItem : public UrlCacheItem
{
public:
    FileCacheItem(Type type, const QUrl &source, const QString &filename, const FileInfo &info);

    QString filename;
    FileInfo info;
};

/**
 * @class ImageCacheItem
 *
 * A structure to define a image item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class ImageCacheItem : public UrlCacheItem
{
public:
    ImageCacheItem(Type type,
                   const QUrl &source,
                   const QString &filename,
                   const ImageInfo &info,
                   const QUrl &thumbnailSource = {},
                   const ImageInfo &thumbnailInfo = {});

    QString filename;
    ImageInfo info;
    QUrl thumbnailSource;
    ImageInfo thumbnailInfo;
};

/**
 * @class VideoCacheItem
 *
 * A structure to define a video item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class VideoCacheItem : public UrlCacheItem
{
public:
    VideoCacheItem(Type type,
                   const QUrl &source,
                   const QString &filename,
                   const VideoInfo &info,
                   const QUrl &thumbnailSource = {},
                   const ImageInfo &thumbnailInfo = {});

    QString filename;
    VideoInfo info;
    QUrl thumbnailSource;
    ImageInfo thumbnailInfo;
};

/**
 * @class AudioCacheItem
 *
 * A structure to define a audio item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class AudioCacheItem : public UrlCacheItem
{
public:
    AudioCacheItem(Type type, const QUrl &source, const QString &filename, const AudioInfo &info);

    QString filename;
    AudioInfo info;
};

/**
 * @class LocationCacheItem
 *
 * A structure to define a location item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class LocationCacheItem : public CacheItem
{
public:
    LocationCacheItem(Type type, qreal latitude, qreal longitude, const QString &asset);

    qreal latitude;
    qreal longitude;
    QString asset;
};

/**
 * @class ReplyCacheItem
 *
 * A structure to define a reply item stored in a Blocks::Cache.
 *
 * @sa Blocks::Cache
 */
class ReplyCacheItem : public CacheItem
{
public:
    ReplyCacheItem(Type type, const QString &id);

    QString id;
};

using CacheItemPtr = std::unique_ptr<CacheItem>;

/**
 * @class Cache
 *
 * A class to cache the contents of a ChatBarMessageContentModel.
 *
 * We can't store the actual content items as the QTextDocuments are attached to
 * text items that may be deleted by the QML engine. Instead we get the contents
 * as a QTextDocumentFragment in a Blocks::CacheItem which can be used to reconstruct the
 * model later.
 *
 * @sa ChatBarMessageContentModel, QTextDocumentFragment, QTextDocument, Blocks::CacheItem
 */
class Cache
{
    using CacheItems = std::vector<std::unique_ptr<CacheItem>>;

public:
    CacheItems::iterator begin();
    CacheItems::iterator end();
    CacheItems::const_iterator begin() const;
    CacheItems::const_iterator end() const;
    CacheItems::const_iterator cbegin() const;
    CacheItems::const_iterator cend() const;

    /**
     * @brief Whether the Cache has any CacheItem in it.
     *
     * @sa CacheItem
     */
    bool empty() const;

    /**
     * @brief Return the CacheItem at the given index.
     *
     * nullptr if i is not a valid index.
     *
     * @sa CacheItem
     */
    const CacheItem *at(qsizetype i) const;

    /**
     * @brief Prepend the given CacheItem to the cache.
     *
     * @sa CacheItem
     */
    void prepend(CacheItemPtr item);

    /**
     * @brief Append the given CacheItem to the cache.
     *
     * @sa CacheItem
     */
    void append(CacheItemPtr item);

    /**
     * @brief Remove the CacheItem at the given index from the cache.
     *
     * @sa CacheItem
     */
    void removeAt(qsizetype i);

    /**
     * @brief Clear the Cache.
     */
    void clear();

    /**
     * @brief Return the contents of the Cache as a single string.
     */
    QString toString() const;

private:
    CacheItems m_items;
};
}
