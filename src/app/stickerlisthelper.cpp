// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "stickerlisthelper.h"

#include <KLocalizedString>

#include "events/imagepackevent.h"
#include "stickerlist.h"

StickerListHelper::StickerListHelper(QObject *parent)
    : QObject(parent)
{
}

NeoChatRoom *StickerListHelper::room() const
{
    return m_room;
}

void StickerListHelper::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
        disconnect(m_room->connection(), nullptr, this, nullptr);
    }
    m_room = room;

    if (m_room) {
        connect(m_room->connection(), &Quotient::Connection::accountDataChanged, this, [this](const QString &type) {
            if (type == "im.ponies.user_emotes"_L1) {
                reloadImages();
            }
        });
    }
    // TODO listen to packs changing
    reloadImages();
    Q_EMIT roomChanged();
}

QList<StickerList *> StickerListHelper::stickerLists() const
{
    return m_lists;
}

void StickerListHelper::reloadImages()
{
    m_lists.clear();

    if (!m_room) {
        return;
    }

    // Load emoticons from the account data
    if (m_room->connection()->hasAccountData("im.ponies.user_emotes"_L1)) {
        auto json = m_room->connection()->accountData("im.ponies.user_emotes"_L1)->contentJson();
        json["pack"_L1] = QJsonObject{{"display_name"_L1, i18nc("As in 'The user's own Stickers'", "Own Stickers")}};
        const auto &content = Quotient::ImagePackEventContent(json);
        if (!content.images.isEmpty()) {
            m_lists.append(new StickerList(
                content,
                [this](const QUrl &source) {
                    return m_room->connection()->makeMediaUrl(source);
                },
                this));
        }
    }

    // Load emoticons from the saved rooms
    const auto &accountData = m_room->connection()->accountData("im.ponies.emote_rooms"_L1);
    if (accountData) {
        const auto &rooms = accountData->contentJson()["rooms"_L1].toObject();
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
                if (const auto &pack = stickerRoom->currentState().get<Quotient::ImagePackEvent>(packKey)) {
                    const auto packContent = pack->content();
                    if ((!packContent.pack || !packContent.pack->usage || packContent.pack->usage->contains("sticker"_L1)) && !packContent.images.isEmpty()) {
                        m_lists.append(new StickerList(
                            packContent,
                            [this](const QUrl &source) {
                                return m_room->connection()->makeMediaUrl(source);
                            },
                            this));
                    }
                }
            }
        }
    }

    // Load emoticons from the current room
    auto events = m_room->currentState().eventsOfType("im.ponies.room_emotes"_L1);
    for (const auto &event : events) {
        const auto imagePackEvent = eventCast<const Quotient::ImagePackEvent>(event);
        if (!imagePackEvent) {
            qWarning() << "Not an image pack event" << imagePackEvent->fullJson();
            continue;
        }
        const auto packContent = imagePackEvent->content();
        if (packContent.pack.has_value()) {
            if (!packContent.pack->usage || packContent.pack->usage->contains("sticker"_L1)) {
                m_lists.append(new StickerList(
                    packContent,
                    [this](const QUrl &source) {
                        return m_room->connection()->makeMediaUrl(source);
                    },
                    this));
            }
        }
    }

    Q_EMIT stickerListsChanged();
}

#include "moc_stickerlisthelper.cpp"
