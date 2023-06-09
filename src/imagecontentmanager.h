// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QHash>
#include <QObject>
#include <QQmlEngine>

#include "events/imagepackevent.h"
#include "neochatroom.h"

#define imageContentManager ImageContentManager::instance()

class ImageContentRole : public QObject
{
    Q_OBJECT
public:
    enum ImageRoles {
        DisplayNameRole = Qt::DisplayRole, /**< The name of the emoji. */
        EmojiRole, /**< The unicode character of the emoji. */
        ShortCodeRole,
        IsCustomRole,
        IsStickerRole,
        IsEmojiRole,
        UsageCountRole,
        HasTonesRole,
    };
    Q_ENUM(ImageRoles);
};

class ImageContentPackRole : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    //! Roles for the various models providing image packs.
    enum ImagePackRoles {
        DisplayNameRole = Qt::DisplayRole, //! Textual desription of the pack.
        IconRole, //! Icon for the pack. For emojis, this is a unicode emoji; For custom emojis and stickers, this is a HTML image.
        IdentifierRole, //! An internal, mostly opaque identifier for the model.
        IsEmojiRole, //! Whether this pack contains emojis (including custom). For the account pack, this is true if the pack contains any emojis; for room
                     //! packs, this *only* considers the pack-level usage parameter
        IsStickerRole, //! Equivalent to IsEmojiRole, but for stickers.
        IsEmptyRole, //! Whether this image pack is empty.
        IsGlobalPackRole, //! Whether this pack is enabled globally.
    };
    Q_ENUM(ImagePackRoles);
};

using RoomImages = QMap<std::pair<QString, QString>, QVector<Quotient::ImagePackEventContent::ImagePackImage>>;

struct Emoji {
    Q_GADGET
    Q_PROPERTY(QString text MEMBER text)
    Q_PROPERTY(QString displayName MEMBER displayName)
    Q_PROPERTY(QString shortName MEMBER shortName)
public:
    QString text;
    QString displayName;
    QString shortName;
};
Q_DECLARE_METATYPE(Emoji)

struct ImagePackDescription {
    enum Type {
        Emoji,
        CustomEmoji,
        Sticker,
        Both,
    };
    QString description;
    QString attribution;
    QString icon;
    Type type;
    // Only relevant for packs coming from rooms
    QString roomId;
    QString stateKey;
};

/**
 * This class manages emojis, custom emojis, and stickers. Because naming things is hard, it has the most generic name possible.
 */
class ImageContentManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    // Returns the global instance of ImageContentManager.
    static ImageContentManager &instance()
    {
        static ImageContentManager _instance;
        return _instance;
    }

    //! Returns a list of emoji packs (categories, e.g., food, smileys, etc.)
    const QVector<ImagePackDescription> &emojiPacks() const;
    //! Returns a map roomId -> stateKey -> description for all image packs that exist in rooms.
    const QMap<QString, QMap<QString, ImagePackDescription>> &roomImagePacks() const;
    //! Returns an list (roomId, stateKey) for all globally enabled room packs.
    //! This is not filtered for rooms or stateKeys that do not exist. This is left to ImagePacksProxyModel
    const QVector<std::pair<QString, QString>> &globalPacks() const;

    //! Returns a map pack key -> [emoji] for all (normal) emojis.
    const QHash<QString, QVector<Emoji>> &emojis() const;

    //! Returns a list of all account images.
    const QVector<Quotient::ImagePackEventContent::ImagePackImage> &accountImages() const;

    //! Returns a map roomId -> stateKey -> [image] of all images part of a room image pack.
    const RoomImages &roomImages() const;

    //! Returns a map emoji -> usage count to be used as an emoji history.
    const QMap<QString, uint32_t> &recentEmojis() const;

    //! Returns the emoji object for the given unicode symbol.
    Emoji emojiForText(const QString &text);

    //! Updates the history when an emoji is used.
    Q_INVOKABLE void emojiUsed(const QString &text);

    QString mxcForShortCode(const QString &shortcode) const;
    QString bodyForShortCode(const QString &shortcode) const;
    bool isEmojiShortCode(const QString &shortCode) const;
    bool isStickerShortCode(const QString &shortCode) const;

    QString accountImagesAvatar() const;

Q_SIGNALS:
    void accountImagesChanged();
    void recentEmojisChanged();
    void roomImagePacksChanged(NeoChatRoom *room);
    void globalPacksChanged();

private:
    // Packs
    QVector<ImagePackDescription> m_emojiPacks;
    // [roomId, stateKey]
    QVector<std::pair<QString, QString>> m_globalPacks;
    // roomId -> stateKey -> description
    QMap<QString, QMap<QString, ImagePackDescription>> m_roomPacks;

    // Emojis
    // pack name -> emojis
    QHash<QString, QVector<Emoji>> m_emojis;
    QVector<Quotient::ImagePackEventContent::ImagePackImage> m_accountImages;
    RoomImages m_roomImages;

    // History
    // emoji -> usage count
    QMap<QString, uint32_t> m_usages;

    // Loads both emojis and emoji packs
    void loadEmojis();
    void loadGlobalPacks();
    void loadRoomImagePacks(NeoChatRoom *room);

    void loadEmojiHistory();

    void loadAccountImages();
    void loadRoomImages();

    ImageContentManager(QObject *parent = nullptr);

    void setupConnection();
    void setupRoom(NeoChatRoom *room);
    void cleanupRoom(NeoChatRoom *room);
};
