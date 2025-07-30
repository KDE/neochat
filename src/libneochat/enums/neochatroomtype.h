// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

#include "neochatroom.h"
#include <Quotient/quotient_common.h>

#include <KLocalizedString>

using namespace Qt::StringLiterals;

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
        ServerNotice, /**< Official messages from the server. */
        Invited, /**< The user has been invited to the room. */
        Favorite, /**< The room is set as a favourite. */
        Direct, /**< The room is a direct chat. */
        Normal, /**< The default category for a joined room. */
        Deprioritized, /**< The room is set as low priority. */
        Space, /**< The room is a space. */
        AddDirect, /**< So we can show the add friend delegate. */
        TypesCount, /**< Number of different types (this should always be last). */
    };
    Q_ENUM(Types);

    static NeoChatRoomType::Types typeForRoom(const NeoChatRoom *room)
    {
        if (room->hasAccountData(u"m.tag"_s) && room->accountData(u"m.tag"_s)->contentPart<QJsonObject>(u"tags"_s).contains(u"m.server_notice"_s)) {
            return NeoChatRoomType::ServerNotice;
        }
        if (room->isSpace()) {
            return NeoChatRoomType::Space;
        }
        if (room->joinState() == Quotient::JoinState::Invite) {
            return NeoChatRoomType::Invited;
        }
        if (room->isFavourite()) {
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
        case NeoChatRoomType::ServerNotice:
            return i18nc("@info Meaning: official information messages from your server", "Server Notices");
        default:
            return {};
        }
    }
    static QString typeIconName(int category)
    {
        switch (category) {
        case NeoChatRoomType::Invited:
            return u"user-invisible"_s;
        case NeoChatRoomType::Favorite:
            return u"favorite"_s;
        case NeoChatRoomType::Direct:
            return u"dialog-messages"_s;
        case NeoChatRoomType::Normal:
            return u"group"_s;
        case NeoChatRoomType::Deprioritized:
            return u"object-order-lower"_s;
        case NeoChatRoomType::Space:
            return u"group"_s;
        default:
            return u"tools-report-bug"_s;
        }
    }
};
