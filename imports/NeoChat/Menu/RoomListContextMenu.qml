/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import NeoChat.Page 1.0

/**
 * Context menu when clicking on a room in the room list
 */
Menu {
    id: root
    property var room

    MenuItem {
        text: i18n("Open in new window")
        onTriggered: roomManager.openWindow(room);
    }

    MenuSeparator {}

    MenuItem {
        text: room.isFavourite ? i18n("Remove from Favourites") : i18n("Add to Favourites")

        onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
    }

    MenuItem {
        text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")

        onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
    }

    MenuItem {
        text: i18n("Mark as Read")

        onTriggered: room.markAllMessagesAsRead()
    }

    MenuSeparator {}

    MenuItem {
        text: i18n("Leave Room")
        onTriggered: {
            if(roomManager.currentRoom == root.room) {
                pageStack.pop()
            }
            room.forget()
        }
    }

    onClosed: destroy()
}
