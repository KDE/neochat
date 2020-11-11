/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma Singleton

import QtQuick 2.14
import NeoChat.Page 1.0

/**
 * Manage opening and close rooms
 */
Item {
    id: openRoomAction

    property var currentRoom: null
    property var pageStack: null

    readonly property bool hasOpenRoom: currentRoom != null

    signal leaveRoom(string room);
    signal openRoom(string room);

    function enterRoom(room) {
        if (currentRoom != null) {
            currentRoom = null;
            pageStack.removePage(pageStack.lastItem);
        }
        pageStack.push(roomPage, {"currentRoom": room});
        currentRoom = room;
    }

    Component {
        id: roomPage

        RoomPage {}
    }
}
