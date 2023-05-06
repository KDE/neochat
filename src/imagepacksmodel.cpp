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
    return m_events.count();
}

QVariant ImagePacksModel::data(const QModelIndex &index, int role) const
{
    const auto &event = m_events[index.row()];
    if (role == DisplayNameRole) {
        if (event.pack->displayName) {
            return *event.pack->displayName;
        }
    }
    if (role == AvatarUrlRole) {
        if (event.pack->avatarUrl) {
#ifdef QUOTIENT_07
            return m_room->connection()->makeMediaUrl(*event.pack->avatarUrl);
#endif
        } else if (!event.images.empty()) {
#ifdef QUOTIENT_07
            return m_room->connection()->makeMediaUrl(event.images[0].url);
#endif
        }
    }
    return {};
}

QHash<int, QByteArray> ImagePacksModel::roleNames() const
{
    return {{DisplayNameRole, "displayName"}, {AvatarUrlRole, "avatarUrl"}, {AttributionRole, "attribution"}, {IdRole, "id"},};
}

NeoChatRoom *ImagePacksModel::room() const
{
    return m_room;
}

void ImagePacksModel::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
    }
    m_room = room;

    beginResetModel();
    m_events.clear();

    // TODO listen to account data changing
    // TODO listen to packs changing
    if (m_room->connection()->hasAccountData("im.ponies.user_emotes"_ls)) {
        auto json = m_room->connection()->accountData("im.ponies.user_emotes"_ls)->contentJson();
        json["pack"] = QJsonObject{
            {"display_name", i18n("Own Stickers")},
        };
        const auto &content = ImagePackEventContent(json);
        if (!content.images.isEmpty()) {
            m_events += ImagePackEventContent(json);
        }
    }
    const auto &accountData = m_room->connection()->accountData("im.ponies.emote_rooms"_ls);
    if (accountData) {
        const auto &rooms = accountData->contentJson()["rooms"_ls].toObject();
        for (const auto &roomId : rooms.keys()) {
            if (roomId == m_room->id()) {
                continue;
            }
            auto packs = rooms[roomId].toObject();
            const auto &stickerRoom = m_room->connection()->room(roomId);
            for (const auto &packKey : packs.keys()) {
#ifdef QUOTIENT_07
                const auto &pack = stickerRoom->currentState().get<ImagePackEvent>(packKey);
                if (pack) {
                    const auto packContent = pack->content();
                    if (!packContent.pack->usage || (packContent.pack->usage->contains("emoticon") && showEmoticons())
                        || (packContent.pack->usage->contains("sticker") && showStickers())) {
                        m_events += packContent;
                    }
                }
#endif
            }
        }
    }
#ifdef QUOTIENT_07
    auto events = m_room->currentState().eventsOfType("im.ponies.room_emotes");
    for (const auto &event : events) {
        auto packContent = eventCast<const ImagePackEvent>(event)->content();
        if (!packContent.pack->usage || (packContent.pack->usage->contains("emoticon") && showEmoticons())
            || (packContent.pack->usage->contains("sticker") && showStickers())) {
            m_events += packContent;
        }
    }
#endif
    endResetModel();
    Q_EMIT roomChanged();
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
QVector<Quotient::ImagePackEventContent::ImagePackImage> ImagePacksModel::images(int index)
{
    if (index < 0 || index >= m_events.size()) {
        return {};
    }
    return m_events[index].images;
}
