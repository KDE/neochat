// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

#include "neochatroom.h"
#include <Quotient/quotient_common.h>

#include <KLocalizedString>

class NeoChatRoomType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Defines the room list categories a room can be assigned.
     */
    enum Types {
        Search = 0, /**< So we can show a search delegate if needed, e.g. collapsed mode. */
        Invited, /**< The user has been invited to the room. */
        Favorite, /**< The room is set as a favourite. */
        Direct, /**< The room is a direct chat. */
        Normal, /**< The default category for a joined room. */
        Deprioritized, /**< The room is set as low priority. */
        Space, /**< The room is a space. */
        AddDirect, /**< So we can show the add friend delegate. */
        TypesCount, /**< Number of different types. */
    };
    Q_ENUM(Types);

    static NeoChatRoomType::Types typeForRoom(const NeoChatRoom *room)
    {
        if (room->isSpace()) {
            return NeoChatRoomType::Space;
        }
        if (room->joinState() == Quotient::JoinState::Invite) {
            return NeoChatRoomType::Invited;
        }
        // HACK for the unit tests
        if (room->isFavourite() || room->property("isFavorite").toBool()) {
            return NeoChatRoomType::Favorite;
        }
        if (room->isLowPriority()) {
            return NeoChatRoomType::Deprioritized;
        }
        if (room->isDirectChat()) {
            return NeoChatRoomType::Direct;
        }
        return NeoChatRoomType::Normal;
    }

    static QString typeName(int category)
    {
        switch (category) {
        case NeoChatRoomType::Invited:
            return i18n("Invited");
        case NeoChatRoomType::Favorite:
            return i18n("Favorite");
        case NeoChatRoomType::Direct:
            return i18n("Friends");
        case NeoChatRoomType::Normal:
            return i18n("Normal");
        case NeoChatRoomType::Deprioritized:
            return i18n("Low priority");
        case NeoChatRoomType::Space:
            return i18n("Spaces");
        case NeoChatRoomType::Search:
            return i18n("Search");
        default:
            return {};
        }
    }
    static QString typeIconName(int category)
    {
        switch (category) {
        case NeoChatRoomType::Invited:
            return QStringLiteral("user-invisible");
        case NeoChatRoomType::Favorite:
            return QStringLiteral("favorite");
        case NeoChatRoomType::Direct:
            return QStringLiteral("dialog-messages");
        case NeoChatRoomType::Normal:
            return QStringLiteral("group");
        case NeoChatRoomType::Deprioritized:
            return QStringLiteral("object-order-lower");
        case NeoChatRoomType::Space:
            return QStringLiteral("group");
        case NeoChatRoomType::Search:
            return QStringLiteral("search");
        default:
            return QStringLiteral("tools-report-bug");
        }
    }
};
