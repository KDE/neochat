// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagecontentmanager.h"

#include <QDebug>
#include <QFile>

#include <KConfigGroup>
#include <KSharedConfig>

#include "controller.h"
#include "events/imagepackevent.h"
#include "neochatroom.h"

#include <Quotient/connection.h>

#define connection Controller::instance().activeConnection()

using namespace Quotient;

ImageContentManager::ImageContentManager(QObject *parent)
    : QObject(parent)
{
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this]() {
        static Connection *oldActiveConnection = nullptr;
        disconnect(oldActiveConnection, nullptr, this, nullptr);
        oldActiveConnection = Controller::instance().activeConnection();
        setupConnection();
    });

    loadEmojis();
    loadEmojiHistory();

    setupConnection();
}

void ImageContentManager::loadEmojis()
{
    QFile file(":/data/emojis.json"_ls);
    file.open(QFile::ReadOnly);
    Q_ASSERT(file.isOpen());
    auto data = QJsonDocument::fromJson(file.readAll()).array();

    for (const auto &emoji : data) {
        // TODO
        // m_emojiPacks += ImagePackDescription{
        //     .description = parts[1],
        //     .attribution = {},
        //     .icon = parts[0],
        //     .type = ImagePackDescription::Emoji,
        //     .roomId = {},
        //     .stateKey = parts[2],
        // };

        m_emojis[u"TODO"_s] += Emoji{
            .text = emoji[u"icon"_s].toString(),
            .displayName = emoji[u"label"_s].toString(),
            .shortName = emoji[u"label"_s].toString(), // TODO
        };
    }
}

void ImageContentManager::loadEmojiHistory()
{
    auto config = KSharedConfig::openStateConfig();
    auto group = config->group("RecentEmojis"_ls);
    for (const auto &key : group.keyList()) {
        m_usages[key] = group.readEntry(key).toInt();
    }
}

void ImageContentManager::setupConnection()
{
    if (!connection) {
        return;
    }
    connect(Controller::instance().activeConnection(), &Connection::accountDataChanged, this, [this](const QString &type) {
        if (type == "im.ponies.user_emotes"_ls) {
            loadAccountImages();
        }
        if (type == "im.ponies.emote_rooms"_ls) {
            loadGlobalPacks();
        }
    });
    loadAccountImages();
    loadGlobalPacks();

    m_roomPacks.clear();

    for (const auto &room : connection->allRooms()) {
        setupRoom(static_cast<NeoChatRoom *>(room));
    }
    connect(connection, &Connection::joinedRoom, this, [this](const auto &room) {
        setupRoom(static_cast<NeoChatRoom *>(room));
    });
    connect(connection, &Connection::leftRoom, this, [this](const auto &room) {
        cleanupRoom(static_cast<NeoChatRoom *>(room));
    });
}

const QVector<ImagePackDescription> &ImageContentManager::emojiPacks() const
{
    return m_emojiPacks;
}

const QHash<QString, QVector<Emoji>> &ImageContentManager::emojis() const
{
    return m_emojis;
}

void ImageContentManager::loadAccountImages()
{
    m_accountImages.clear();
    if (connection->hasAccountData("im.ponies.user_emotes"_ls)) {
        m_accountImages = ImagePackEventContent(connection->accountData("im.ponies.user_emotes"_ls)->contentJson()).images;
    }
    Q_EMIT accountImagesChanged();
}

const QVector<ImagePackEventContent::ImagePackImage> &ImageContentManager::accountImages() const
{
    return m_accountImages;
}

void ImageContentManager::emojiUsed(const QString &text)
{
    if (!m_usages.contains(text)) {
        m_usages[text] = 0;
    }
    m_usages[text]++;
    Q_EMIT recentEmojisChanged();
    auto config = KSharedConfig::openStateConfig();
    auto group = config->group("RecentEmojis"_ls);
    for (const auto &key : m_usages.keys()) {
        group.writeEntry(key, m_usages[key]);
    }
}

Emoji ImageContentManager::emojiForText(const QString &text)
{
    for (const auto &category : m_emojis.values()) {
        for (const auto &emoji : category) {
            if (emoji.text == text) {
                return emoji;
            }
        }
    }
    const auto &withSelector = QString::fromUtf8(text.toUtf8() + QByteArrayLiteral("\xEF\xB8\x8F"));
    for (const auto &category : m_emojis.values()) {
        for (const auto &emoji : category) {
            if (emoji.text == withSelector) {
                return emoji;
            }
        }
    }
    return {};
}

const QMap<QString, uint32_t> &ImageContentManager::recentEmojis() const
{
    return m_usages;
}

const QMap<QString, QMap<QString, ImagePackDescription>> &ImageContentManager::roomImagePacks() const
{
    return m_roomPacks;
}

void ImageContentManager::loadRoomImagePacks(NeoChatRoom *room)
{
    const auto &events = room->currentState().eventsOfType("im.ponies.room_emotes"_ls);
    m_roomPacks[room->id()].clear();
    for (const auto &event : events) {
        auto content = ImagePackEventContent(event->contentJson());
        auto avatarMxc = event->contentPart<QJsonObject>("pack"_ls)["avatar_url"_ls].toString();
        if (avatarMxc.isEmpty()) {
            const auto &images = event->contentPart<QJsonObject>("images"_ls);
            if (images.size() > 0) {
                avatarMxc = images[images.keys()[0]]["url"_ls].toString();
            }
        }
        const auto &avatarUrl = avatarMxc.isEmpty() ? QString() : Controller::instance().activeConnection()->makeMediaUrl(QUrl(avatarMxc)).toString();

        ImagePackDescription::Type type = ImagePackDescription::Both;
        if (!content.pack || !content.pack->usage || content.pack->usage->isEmpty()
            || (content.pack->usage->contains("emoticon"_ls) && content.pack->usage->contains("sticker"_ls))) {
            type = ImagePackDescription::Both;
        } else if (content.pack->usage->contains("sticker"_ls)) {
            type = ImagePackDescription::Sticker;
        } else {
            type = ImagePackDescription::CustomEmoji;
        }

        m_roomPacks[room->id()][event->stateKey()] = ImagePackDescription{
            .description = event->contentPart<QJsonObject>("pack"_ls)["display_name"_ls].toString(),
            .attribution = {},
            .icon = QStringLiteral("<img src=\"%1\" width=\"32\" height=\"32\"/>").arg(avatarUrl),
            .type = type,
            .roomId = room->id(),
            .stateKey = event->stateKey(),
        };
        m_roomImages[{room->id(), event->stateKey()}] = content.images;
    }
    Q_EMIT roomImagePacksChanged(room);
}

const RoomImages &ImageContentManager::roomImages() const
{
    return m_roomImages;
}

const QVector<std::pair<QString, QString>> &ImageContentManager::globalPacks() const
{
    return m_globalPacks;
}

void ImageContentManager::loadGlobalPacks()
{
    if (!connection->hasAccountData("im.ponies.emote_rooms"_ls)) {
        return;
    }
    m_globalPacks.clear();
    const auto &rooms = Controller::instance().activeConnection()->accountData("im.ponies.emote_rooms"_ls)->contentPart<QJsonObject>("rooms"_ls);
    for (const auto &roomId : rooms.keys()) {
        for (const auto &stateKey : rooms[roomId].toObject().keys()) {
            m_globalPacks += {roomId, stateKey};
        }
    }
    Q_EMIT globalPacksChanged();
}

void ImageContentManager::setupRoom(NeoChatRoom *room)
{
    connect(room, &Room::changed, this, [this, room]() {
        loadRoomImagePacks(room);
    });
    loadRoomImagePacks(room);
}

void ImageContentManager::cleanupRoom(NeoChatRoom *room)
{
    m_roomPacks.remove(room->id());
    Q_EMIT roomImagePacksChanged(room);
}

QString ImageContentManager::mxcForShortCode(const QString &shortcode) const
{
    for (const auto &image : m_accountImages) {
        if (image.shortcode == shortcode) {
            return Controller::instance().activeConnection()->makeMediaUrl(image.url).toString();
        }
    }
    for (const auto &id : m_roomImages.keys()) {
        for (const auto &image : m_roomImages[id]) {
            if (image.shortcode == shortcode) {
                return Controller::instance().activeConnection()->makeMediaUrl(image.url).toString();
            }
        }
    }
    return {};
}

QString ImageContentManager::bodyForShortCode(const QString &shortcode) const
{
    for (const auto &image : m_accountImages) {
        if (image.shortcode == shortcode) {
            return image.body.value_or(QString());
        }
    }
    for (const auto &id : m_roomImages.keys()) {
        for (const auto &image : m_roomImages[id]) {
            if (image.shortcode == shortcode) {
                return image.body.value_or(QString());
            }
        }
    }
    return {};
}

bool ImageContentManager::isEmojiShortCode(const QString &shortCode) const
{
    for (const auto &image : m_accountImages) {
        if (image.shortcode == shortCode) {
            return !image.usage || image.usage->isEmpty() || image.usage->contains("emoticon"_ls);
        }
    }
    for (const auto &id : m_roomImages.keys()) {
        for (const auto &image : m_roomImages[id]) {
            if (image.shortcode == shortCode) {
                const auto pack = m_roomPacks[id.first][id.second];
                return pack.type == ImagePackDescription::Emoji || pack.type == ImagePackDescription::Both;
            }
        }
    }
    return true;
}

bool ImageContentManager::isStickerShortCode(const QString &shortCode) const
{
    for (const auto &image : m_accountImages) {
        if (image.shortcode == shortCode) {
            return !image.usage || image.usage->isEmpty() || image.usage->contains("sticker"_ls);
        }
    }
    for (const auto &id : m_roomImages.keys()) {
        for (const auto &image : m_roomImages[id]) {
            if (image.shortcode == shortCode) {
                const auto pack = m_roomPacks[id.first][id.second];
                return pack.type == ImagePackDescription::Sticker || pack.type == ImagePackDescription::Both;
            }
        }
    }
    return true;
}

QString ImageContentManager::accountImagesAvatar() const
{
    if (!connection->hasAccountData("im.ponies.user_emotes"_ls)) {
        return {};
    }
    const auto &event = ImagePackEventContent(connection->accountData("im.ponies.user_emotes"_ls)->contentJson());
    QString avatarUrl;
    if (event.pack) {
        avatarUrl = event.pack->avatarUrl.value_or(QUrl()).toString();
    }
    if (avatarUrl.isEmpty()) {
        //TODO avatarUrl = Controller::instance().activeConnection()->user()->avatarUrl().toString();
    }
    if (avatarUrl.isEmpty()) {
        avatarUrl = event.images[0].url.toString();
    }
    return QStringLiteral("👤");
}
