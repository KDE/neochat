// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "blockcache.h"
#include "chattextitemhelper.h"
#include "enums/blocktype.h"
#include "fileinfo.h"
#include "models/itinerarymodel.h"
#include "models/reactionmodel.h"
#include "neochatroom.h"

namespace Blocks
{
/**
 * @class Block
 *
 * The base Block from which all others are derived.
 *
 * This can be used if only a type is required, i.e. the block being shown has no
 * variable content. Otherwise one of the inherited classes should be used.
 *
 * Virtual functions are provided to allow the following:
 *  - toCacheItem() - for conversion to a ChacheItem for storage
 *    the inherited class and it's properties.
 *  - operator==() - to overide default equality (which is normally just checking that
 *    the type is the same).
 */
class Block : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The block type.
     */
    Q_PROPERTY(Blocks::Type type READ type WRITE setType NOTIFY typeChanged)

public:
    Block(Type type, QObject *parent);
    Block(CacheItem *item, QObject *parent);

    [[nodiscard]] Type type() const;
    void setType(Type type);

    [[nodiscard]] virtual CacheItemPtr toCacheItem() const;

    [[nodiscard]] virtual QVariant toVariant() const;

    virtual bool operator==(const Block &right) const;

    [[nodiscard]] bool isEmpty() const;

Q_SIGNALS:
    void typeChanged();

private:
    Type m_type = Other;
};

/**
 * @class BasicTextBlock
 *
 * A block to visualize a simple text string.
 *
 * This block is designed for components which just need to display a simple string
 * normally a single line of plain text. If more complex display and/or editability
 * is required then TextBlock should be used.
 */
class BasicTextBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The display text of the block.
     */
    Q_PROPERTY(QString display READ display CONSTANT)

public:
    BasicTextBlock(Type type, const QString &display, QObject *parent);
    BasicTextBlock(BasicTextCacheItem *item, QObject *parent);

    [[nodiscard]] QString display() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QString m_display;
};

/**
 * @class TextBlock
 *
 * A block to visualize a complex text.
 *
 * This manages the text using a ChatTextItemHelper which is a wrapper around the
 * qml visual component. The intial text is inserted directly into the QTextDocument
 * of the visual item.
 */
class TextBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The ChatTextItemHelper managing the visual component.
     */
    Q_PROPERTY(ChatTextItemHelper *item READ item CONSTANT)

    /**
     * @brief Whether the block contains a spoiler tag.
     */
    Q_PROPERTY(bool hasSpoiler READ hasSpoiler CONSTANT)

    /**
     * @brief Whether the spoiler has been revealed (if it exists).
     */
    Q_PROPERTY(bool spoilerRevealed READ spoilerRevealed WRITE setSpoilerRevealed NOTIFY spoilerRevealedChanged)

public:
    TextBlock(Type type, const QTextDocumentFragment &content, bool hasSpoiler, QObject *parent);
    TextBlock(TextCacheItem *item, QObject *parent);

    ChatTextItemHelper *item() const;

    [[nodiscard]] bool hasSpoiler() const;
    [[nodiscard]] bool spoilerRevealed() const;
    void setSpoilerRevealed(bool spoilerRevealed);

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

Q_SIGNALS:
    void spoilerRevealedChanged();

private:
    ChatTextItemHelper *m_item = nullptr;
    bool m_hasSpoiler = false;
    bool m_spoilerRevealed = false;
};

/**
 * @class CodeBlock
 *
 * A block to visualize a code text.
 *
 * This is the same as TextBlock but with additional code language support.
 */
class CodeBlock : public TextBlock
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The code language for the block.
     */
    Q_PROPERTY(QString language READ language CONSTANT)

public:
    CodeBlock(Type type, const QTextDocumentFragment &content, const QString &language, QObject *parent);
    CodeBlock(CodeCacheItem *item, QObject *parent);

    [[nodiscard]] QString language() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QString m_language;
};

/**
 * @class UrlBlock
 *
 * A block to visualize URL based content.
 */
class UrlBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The file source.
     */
    Q_PROPERTY(QUrl source READ source CONSTANT)

public:
    UrlBlock(Type type, const QUrl &source, QObject *parent);
    UrlBlock(UrlCacheItem *item, QObject *parent);

    [[nodiscard]] QUrl source() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QUrl m_source;
};

/**
 * @class FileBlock
 *
 * A block to visualize a file.
 */
class FileBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The info for the file.
     */
    Q_PROPERTY(QString filename READ filename CONSTANT)

    /**
     * @brief The FileInfo for the file.
     *
     * @sa FileInfo
     */
    Q_PROPERTY(FileInfo info READ info CONSTANT)

public:
    FileBlock(Type type, const QUrl &source, const QString &filename, const FileInfo &info, QObject *parent);
    FileBlock(FileCacheItem *item, QObject *parent);

    QString filename() const;
    const FileInfo &info() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QString m_filename;
    FileInfo m_info;
};

/**
 * @class ImageBlock
 *
 * A block to visualize an image.
 */
class ImageBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The filename for the image.
     */
    Q_PROPERTY(QString filename READ filename CONSTANT)

    /**
     * @brief The ImageInfo for the image.
     *
     * @sa ImageInfo
     */
    Q_PROPERTY(ImageInfo info READ info CONSTANT)

    /**
     * @brief The source for the image thumbnail if any.
     */
    Q_PROPERTY(QUrl thumbnailSource READ thumbnailSource CONSTANT)

    /**
     * @brief The ImageInfo for the image thumbnail if any.
     *
     * @sa ImageInfo
     */
    Q_PROPERTY(ImageInfo thumbnailInfo READ thumbnailInfo CONSTANT)

public:
    ImageBlock(Type type,
               const QUrl &source,
               const QString &filename,
               const ImageInfo &info,
               const QUrl &thumbnailSource,
               const ImageInfo &thumbnailInfo,
               QObject *parent);
    ImageBlock(ImageCacheItem *item, QObject *parent);

    QString filename() const;
    const ImageInfo &info() const;
    QUrl thumbnailSource() const;
    const ImageInfo &thumbnailInfo() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QString m_filename;
    ImageInfo m_info;
    QUrl m_thumbnailSource;
    ImageInfo m_thumbnailInfo;
};

/**
 * @class VideoBlock
 *
 * A block to visualize a video.
 */
class VideoBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The filename for the video.
     */
    Q_PROPERTY(QString filename READ filename CONSTANT)

    /**
     * @brief The VideoInfo for the video.
     *
     * @sa VideoInfo
     */
    Q_PROPERTY(VideoInfo info READ info CONSTANT)

    /**
     * @brief The source for the video thumbnail if any.
     */
    Q_PROPERTY(QUrl thumbnailSource READ thumbnailSource CONSTANT)

    /**
     * @brief The ImageInfo for the video thumbnail if any.
     *
     * \sa ImageInfo
     */
    Q_PROPERTY(ImageInfo thumbnailInfo READ thumbnailInfo CONSTANT)

public:
    VideoBlock(Type type,
               const QUrl &source,
               const QString &filename,
               const VideoInfo &info,
               const QUrl &thumbnailSource,
               const ImageInfo &thumbnailInfo,
               QObject *parent);
    VideoBlock(VideoCacheItem *item, QObject *parent);

    QString filename() const;
    const VideoInfo &info() const;
    QUrl thumbnailSource() const;
    const ImageInfo &thumbnailInfo() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QString m_filename;
    VideoInfo m_info;
    QUrl m_thumbnailSource;
    ImageInfo m_thumbnailInfo;
};

/**
 * @class AudioBlock
 *
 * A block to visualize an audio file.
 */
class AudioBlock : public UrlBlock
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The filename for the audio file.
     */
    Q_PROPERTY(QString filename READ filename CONSTANT)

    /**
     * @brief The AudioInfo for the audio file.
     *
     * @sa AudioInfo
     */
    Q_PROPERTY(AudioInfo info READ info CONSTANT)

public:
    AudioBlock(Type type, const QUrl &source, const QString &filename, const AudioInfo &info, QObject *parent);
    AudioBlock(AudioCacheItem *item, QObject *parent);

    QString filename() const;
    const AudioInfo &info() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    QString m_filename;
    AudioInfo m_info;
};

/**
 * @class ItineraryBlock
 *
 * A block to help visualize a message file that is an itinerary item.
 *
 * The ItineraryBlock will create and manage a ItineraryModel.
 */
class ItineraryBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The model containing the list of reactions.
     */
    Q_PROPERTY(ItineraryModel *model READ model CONSTANT)

public:
    ItineraryBlock(Type type, const QUrl &source, QObject *parent);

    ItineraryModel *model() const;

private:
    ItineraryModel *m_model;
};

/**
 * @class LocationBlock
 *
 * A block to visualize a location.
 */
class LocationBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The latitude of the location.
     */
    Q_PROPERTY(qreal latitude READ latitude CONSTANT)

    /**
     * @brief The longitude of the location.
     */
    Q_PROPERTY(qreal longitude READ longitude CONSTANT)

    /**
     * @brief The asset type of the location.
     */
    Q_PROPERTY(QString asset READ asset CONSTANT)

public:
    LocationBlock(Type type, qreal latitude, qreal longitude, const QString &asset, QObject *parent);
    LocationBlock(LocationCacheItem *item, QObject *parent);

    qreal latitude() const;
    qreal longitude() const;
    QString asset() const;

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

private:
    qreal m_latitude = 0.0;
    qreal m_longitude = 0.0;
    QString m_asset;
};

/**
 * @class ReplyBlock
 *
 * A block to visualize a reply.
 */
class ReplyBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The ID of the event being replied to.
     */
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)

public:
    ReplyBlock(Type type, const QString &id, QObject *parent);
    ReplyBlock(ReplyCacheItem *item, QObject *parent);

    QString id() const;
    void setId(const QString &id);

    [[nodiscard]] CacheItemPtr toCacheItem() const override;

Q_SIGNALS:
    void idChanged();

private:
    QString m_id;
};

/**
 * @class ReactionBlock
 *
 * A block to help visualize reactions to a message.
 *
 * The ReactionBlock will create and manage a ReactionModel.
 */
class ReactionBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The model containing the list of reactions.
     */
    Q_PROPERTY(ReactionModel *model READ model CONSTANT)

public:
    ReactionBlock(Type type, NeoChatRoom *room, const QString &eventId, QObject *parent);

    ReactionModel *model() const;

private:
    ReactionModel *m_model;
};

/**
 * @class ChatBarBlock
 *
 * A block to help visualize a chat bar in a message.
 */
class ChatBarBlock : public Block
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The model containing the list of reactions.
     */
    Q_PROPERTY(bool isEditing READ isEditing CONSTANT)

    /**
     * @brief The model containing the list of reactions.
     */
    Q_PROPERTY(QString threadRootId READ threadRootId CONSTANT)

public:
    ChatBarBlock(Type type, bool isEditing, const QString &threadRootId, QObject *parent);

    [[nodiscard]] bool isEditing() const;
    [[nodiscard]] QString threadRootId() const;

private:
    bool m_isEditing = false;
    QString m_threadRootId = {};
};

using BlockPtrs = std::vector<Blocks::Block *>;
using BlockPtrsIt = BlockPtrs::iterator;
}
