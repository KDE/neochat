// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <KLocalizedString>

#include <Integral/lib.rs.h>

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
    enum Type {
        Invited, /**< The user has been invited to the room. */
        Favorite, /**< The room is set as a favourite. */
        Direct, /**< The room is a direct chat. */
        Normal, /**< The default category for a joined room. */
        Deprioritized, /**< The room is set as low priority. */
        Space, /**< The room is a space. */
        AddDirect, /**< So we can show the add friend delegate. */
        TypesCount, /**< Number of different types (this should always be last). */
    };
    Q_ENUM(Type);

    static NeoChatRoomType::Type typeForRoom(rust::Box<sdk::RoomListRoom> room)
    {
        if (room->is_space()) {
            return NeoChatRoomType::Space;
        }
        if (room->state() == 2) {
            return NeoChatRoomType::Invited;
        }
        if (room->is_favourite()) {
            return NeoChatRoomType::Favorite;
        }
        if (room->is_low_priority()) {
            return NeoChatRoomType::Deprioritized;
        }
        if (false /*room->isDirectChat()*/) {
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
