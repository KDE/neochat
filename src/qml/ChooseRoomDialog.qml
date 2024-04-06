// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title", "Choose a Room")

    signal chosen(string roomId)

    required property NeoChatConnection connection

    header: Kirigami.SearchField {
        onTextChanged: RoomManager.sortFilterRoomListModel.filterText = text
    }

    ListView {
        model: RoomManager.sortFilterRoomListModel
        delegate: RoomDelegate {
            id: roomDelegate
            onClicked: {
                root.chosen(roomDelegate.currentRoom.id);
            }
            connection: root.connection
        }
    }

    Component.onCompleted: Qt.callLater(() => header.forceActiveFocus())
}
