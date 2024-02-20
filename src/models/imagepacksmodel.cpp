// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "imagepacksmodel.h"
#include "neochatroom.h"

#include <KLocalizedString>

using namespace Quotient;

ImagePacksModel::ImagePacksModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ImagePacksModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_events.count();
}

QVariant ImagePacksModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    if (row < 0 || row >= m_events.size()) {
        return {};
    }
    const auto &event = m_events[row];
    if (role == DisplayNameRole) {
        if (event.pack->displayName) {
            return *event.pack->displayName;
        }
    }
    if (role == AvatarUrlRole) {
        if (event.pack->avatarUrl) {
            return m_room->connection()->makeMediaUrl(*event.pack->avatarUrl);
        } else if (!event.images.empty()) {
            return m_room->connection()->makeMediaUrl(event.images[0].url);
        }
    }
    return {};
}

QHash<int, QByteArray> ImagePacksModel::roleNames() const
{
    return {
        {DisplayNameRole, "displayName"},
        {AvatarUrlRole, "avatarUrl"},
        {AttributionRole, "attribution"},
        {IdRole, "id"},
    };
}

NeoChatRoom *ImagePacksModel::room() const
{
    return m_room;
}

void ImagePacksModel::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
        disconnect(m_room->connection(), nullptr, this, nullptr);
    }
    m_room = room;

    if (m_room) {
        connect(m_room->connection(), &Connection::accountDataChanged, this, [this](const QString &type) {
            if (type == "im.ponies.user_emotes"_ls) {
                reloadImages();
            }
        });
    }
    // TODO listen to packs changing
    reloadImages();
    Q_EMIT roomChanged();
}

void ImagePacksModel::reloadImages()
{
    if (!m_room) {
        return;
    }
    beginResetModel();
    m_events.clear();

    // Load emoticons from the account data
    if (m_room->connection()->hasAccountData("im.ponies.user_emotes"_ls)) {
        auto json = m_room->connection()->accountData("im.ponies.user_emotes"_ls)->contentJson();
        json["pack"_ls] = QJsonObject{
            {"display_name"_ls,
             m_showStickers ? i18nc("As in 'The user's own Stickers'", "Own Stickers") : i18nc("As in 'The user's own emojis", "Own Emojis")},
        };
        const auto &content = ImagePackEventContent(json);
        if (!content.images.isEmpty()) {
            m_events += ImagePackEventContent(json);
        }
    }

    // Load emoticons from the saved rooms
    const auto &accountData = m_room->connection()->accountData("im.ponies.emote_rooms"_ls);
    if (accountData) {
        const auto &rooms = accountData->contentJson()["rooms"_ls].toObject();
        for (const auto &roomId : rooms.keys()) {
            if (roomId == m_room->id()) {
                continue;
            }
            auto packs = rooms[roomId].toObject();
            const auto &stickerRoom = m_room->connection()->room(roomId);
            if (!stickerRoom) {
                continue;
            }
            for (const auto &packKey : packs.keys()) {
                if (const auto &pack = stickerRoom->currentState().get<ImagePackEvent>(packKey)) {
                    const auto packContent = pack->content();
                    if ((!packContent.pack || !packContent.pack->usage || (packContent.pack->usage->contains("emoticon"_ls) && showEmoticons())
                         || (packContent.pack->usage->contains("sticker"_ls) && showStickers()))
                        && !packContent.images.isEmpty()) {
                        m_events += packContent;
                    }
                }
            }
        }
    }

    // Load emoticons from the current room
    auto events = m_room->currentState().eventsOfType("im.ponies.room_emotes"_ls);
    for (const auto &event : events) {
        auto packContent = eventCast<const ImagePackEvent>(event)->content();
        if (packContent.pack.has_value()) {
            if (!packContent.pack->usage || (packContent.pack->usage->contains("emoticon"_ls) && showEmoticons())
                || (packContent.pack->usage->contains("sticker"_ls) && showStickers())) {
                m_events += packContent;
            }
        }
    }
    Q_EMIT imagesLoaded();
    endResetModel();
}

bool ImagePacksModel::showStickers() const
{
    return m_showStickers;
}

void ImagePacksModel::setShowStickers(bool showStickers)
{
    m_showStickers = showStickers;
    Q_EMIT showStickersChanged();
}

bool ImagePacksModel::showEmoticons() const
{
    return m_showEmoticons;
}

void ImagePacksModel::setShowEmoticons(bool showEmoticons)
{
    m_showEmoticons = showEmoticons;
    Q_EMIT showEmoticonsChanged();
}
QList<Quotient::ImagePackEventContent::ImagePackImage> ImagePacksModel::images(int index)
{
    if (index < 0 || index >= m_events.size()) {
        return {};
    }
    return m_events[index].images;
}

#include "moc_imagepacksmodel.cpp"
