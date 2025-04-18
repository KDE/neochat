// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

import org.kde.neochat

SearchPage {
    id: root

    title: i18nc("@title", "Choose a Room")

    showSearchButton: false

    signal chosen(string roomId)

    required property NeoChatConnection connection

    model: RoomManager.sortFilterRoomListModel
    modelDelegate: RoomDelegate {
        onClicked: {
            root.chosen(currentRoom.id);
            root.closeDialog();
        }
        connection: root.connection
        openOnClick: false
    }
}
