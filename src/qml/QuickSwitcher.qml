// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.neochat

QQC2.Dialog {
    id: root

    required property NeoChatConnection connection

    parent: applicationWindow().overlay
    width: Math.min(700, parent.width)
    height: 400

    leftPadding: 0
    rightPadding: 0
    bottomPadding: 1
    topPadding: 0

    anchors.centerIn: applicationWindow().overlay

    Shortcut {
        sequence: "Ctrl+K"
        onActivated: root.open()
    }

    onVisibleChanged: {
        if (!visible) {
            return;
        }
        searchField.forceActiveFocus();
        searchField.text = "";
        roomList.currentIndex = 0;
    }

    header: Kirigami.SearchField {
        id: searchField
        Keys.onDownPressed: {
            roomList.forceActiveFocus();
            if (roomList.currentIndex < roomList.count - 1) {
                roomList.currentIndex++;
            } else {
                roomList.currentIndex = 0;
            }
        }
        Keys.onUpPressed: {
            if (roomList.currentIndex === 0) {
                roomList.currentIndex = roomList.count - 1;
            } else {
                roomList.currentIndex--;
            }
        }
        Keys.onEnterPressed: {
            RoomManager.resolveResource(roomList.currentItem.currentRoom.id);
            root.close();
        }
        Keys.onReturnPressed: {
            RoomManager.resolveResource(roomList.currentItem.currentRoom.id);
            root.close();
        }
        focusSequence: ""
        onTextChanged: RoomManager.sortFilterRoomListModel.filterText = text
    }

    QQC2.ScrollView {
        anchors.fill: parent
        clip: true

        Keys.forwardTo: searchField

        ListView {
            id: roomList

            currentIndex: 0
            highlightMoveDuration: 200
            Keys.forwardTo: searchField
            keyNavigationEnabled: true
            model: RoomManager.sortFilterRoomListModel

            delegate: RoomDelegate {
                connection: root.connection
                onClicked: root.close()
            }
        }
    }
}
