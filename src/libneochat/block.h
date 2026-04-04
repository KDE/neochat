// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "blockcache.h"
#include "chattextitemhelper.h"
#include "enums/blocktype.h"
#include "fileinfo.h"

namespace Blocks
{
class Block : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The block type.
     */
    Q_PROPERTY(Blocks::Type type MEMBER type CONSTANT)

public:
    Block(QObject *parent = nullptr);
    Block(Type type, QObject *parent = nullptr);
    Block(CacheItem *item, QObject *parent = nullptr);

    Type type = Other;

    virtual CacheItemPtr toCacheItem() const;

    virtual QVariant toVariant() const;

    virtual bool operator==(const Block &right) const
    {
        return type == right.type;
    }

    bool isEmpty() const
    {
        return type == Other;
    }
};

class TextBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The block type.
     */
    Q_PROPERTY(ChatTextItemHelper *item READ item CONSTANT)

    /**
     * @brief Whether the block contains a spoiler tag.
     */
    Q_PROPERTY(bool hasSpoiler MEMBER hasSpoiler CONSTANT)

    /**
     * @brief Whether the a spoiler if it exists has been revealed.
     */
    Q_PROPERTY(bool spoilerRevealed MEMBER spoilerRevealed CONSTANT)

public:
    TextBlock(QObject *parent = nullptr);
    TextBlock(Type type, const QTextDocumentFragment &content, bool hasSpoiler = false, QObject *parent = nullptr);
    TextBlock(TextCacheItem *item, QObject *parent = nullptr);

    ChatTextItemHelper *item() const;

    bool hasSpoiler;
    bool spoilerRevealed = false;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;

private:
    ChatTextItemHelper *m_item;
};

class CodeBlock : public TextBlock
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The block type.
     */
    Q_PROPERTY(ChatTextItemHelper *item READ item CONSTANT)

    /**
     * @brief The code language for the block.
     */
    Q_PROPERTY(QString language MEMBER language CONSTANT)

public:
    CodeBlock(QObject *parent = nullptr);
    CodeBlock(Type type, const QTextDocumentFragment &content, const QString &language, QObject *parent = nullptr);
    CodeBlock(CodeCacheItem *item, QObject *parent = nullptr);

    QString language;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;

private:
    ChatTextItemHelper *m_item;
};

class UrlBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The file source.
     */
    Q_PROPERTY(QUrl source MEMBER source CONSTANT)

public:
    UrlBlock(QObject *parent = nullptr);
    UrlBlock(Type type, const QUrl &source, QObject *parent = nullptr);
    UrlBlock(UrlCacheItem *item, QObject *parent = nullptr);

    QUrl source;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

class FileBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(QString filename MEMBER filename CONSTANT)

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(FileInfo info MEMBER info CONSTANT)

public:
    FileBlock(QObject *parent = nullptr);
    FileBlock(Type type, const QUrl &source, const QString &filename, const FileInfo &info, QObject *parent = nullptr);
    FileBlock(FileCacheItem *item, QObject *parent = nullptr);

    QString filename;
    FileInfo info;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

class ImageBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(QString filename MEMBER filename CONSTANT)

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(ImageInfo info MEMBER info CONSTANT)

    /**
     * @brief The source for the file thumbnail if any.
     */
    Q_PROPERTY(QUrl thumbnailSource MEMBER thumbnailSource CONSTANT)

    /**
     * @brief The info for the file thumbnail if any.
     */
    Q_PROPERTY(ImageInfo thumbnailInfo MEMBER thumbnailInfo CONSTANT)

public:
    ImageBlock(QObject *parent = nullptr);
    ImageBlock(Type type,
               const QUrl &source,
               const QString &filename,
               const ImageInfo &info,
               const QUrl &thumbnailSource = {},
               const ImageInfo &thumbnailInfo = {},
               QObject *parent = nullptr);
    ImageBlock(ImageCacheItem *item, QObject *parent = nullptr);

    QString filename;
    ImageInfo info;
    QUrl thumbnailSource;
    ImageInfo thumbnailInfo;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

class VideoBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(QString filename MEMBER filename CONSTANT)

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(VideoInfo info MEMBER info CONSTANT)

    /**
     * @brief The source for the file thumbnail if any.
     */
    Q_PROPERTY(QUrl thumbnailSource MEMBER thumbnailSource CONSTANT)

    /**
     * @brief The info for the file thumbnail if any.
     */
    Q_PROPERTY(ImageInfo thumbnailInfo MEMBER thumbnailInfo CONSTANT)

public:
    VideoBlock(QObject *parent = nullptr);
    VideoBlock(Type type,
               const QUrl &source,
               const QString &filename,
               const VideoInfo &info,
               const QUrl &thumbnailSource = {},
               const ImageInfo &thumbnailInfo = {},
               QObject *parent = nullptr);
    VideoBlock(VideoCacheItem *item, QObject *parent = nullptr);

    QString filename;
    VideoInfo info;
    QUrl thumbnailSource;
    ImageInfo thumbnailInfo;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

class AudioBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(QString filename MEMBER filename CONSTANT)

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(AudioInfo info MEMBER info CONSTANT)

public:
    AudioBlock(QObject *parent = nullptr);
    AudioBlock(Type type, const QUrl &source, const QString &filename, const AudioInfo &info, QObject *parent = nullptr);
    AudioBlock(AudioCacheItem *item, QObject *parent = nullptr);

    QString filename;
    AudioInfo info;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

class LocationBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The latitude of the location.
     */
    Q_PROPERTY(qreal latitude MEMBER latitude CONSTANT)

    /**
     * @brief The longitude of the location.
     */
    Q_PROPERTY(qreal longitude MEMBER longitude CONSTANT)

    /**
     * @brief The asset type of the location.
     */
    Q_PROPERTY(QString assest MEMBER asset CONSTANT)

public:
    LocationBlock(QObject *parent = nullptr);
    LocationBlock(Type type, qreal latitude, qreal longitude, const QString &asset, QObject *parent = nullptr);
    LocationBlock(LocationCacheItem *item, QObject *parent = nullptr);

    qreal latitude;
    qreal longitude;
    QString asset;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

class ReplyBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The video source.
     */
    Q_PROPERTY(QString id MEMBER id CONSTANT)

public:
    ReplyBlock(QObject *parent = nullptr);
    ReplyBlock(Type type, const QString &id, QObject *parent = nullptr);
    ReplyBlock(ReplyCacheItem *item, QObject *parent = nullptr);

    QString id;

    CacheItemPtr toCacheItem() const override;

    QVariant toVariant() const override;
};

using BlockPtr = std::unique_ptr<Blocks::Block>;
using BlockPtrs = std::vector<BlockPtr>;
using BlockPtrsIt = BlockPtrs::iterator;

template<typename BlockT>
concept BlockClass = std::derived_from<BlockT, Block>;

template<BlockClass BlockT, typename... ArgTs>
BlockPtr makeBlock(ArgTs &&...args)
{
    return std::make_unique<BlockT>(std::forward<ArgTs>(args)...);
}
}
