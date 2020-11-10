/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

/**
 * Context menu when clicking on a room in the room list
 */
Menu {
    id: root
    property var room

    MenuItem {
        text: i18n("Favourite")
        checkable: true
        checked: room.isFavourite

        onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
    }

    MenuItem {
        text: i18n("Deprioritize")
        checkable: true
        checked: room.isLowPriority

        onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
    }

    MenuSeparator {}

    MenuItem {
        text: i18n("Mark as Read")

        onTriggered: room.markAllMessagesAsRead()
    }

    MenuItem {
        text: i18n("Leave Room")
        onTriggered: room.forget()
    }

    onClosed: destroy()
}
