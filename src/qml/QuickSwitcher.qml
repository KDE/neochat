// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kitemmodels

import org.kde.neochat

QQC2.Dialog {
    id: root

    required property NeoChatConnection connection

    parent: QQC2.Overlay.overlay
    width: Math.min(700, parent.width)
    height: 400

    leftPadding: 0
    rightPadding: 0
    bottomPadding: 1 // HACK fix graphical glitch in the bottom
    topPadding: 0

    anchors.centerIn: parent

    modal: true

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

    background: DialogRoundedBackground {}

    contentItem: ColumnLayout {
        spacing: 0

        Kirigami.SearchField {
            id: searchField

            Layout.fillWidth: true

            background: null

            Layout.margins: Kirigami.Units.smallSpacing

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

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        QQC2.ScrollView {
            clip: true

            Layout.fillWidth: true
            Layout.fillHeight: true
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
                    showConfigure: false
                }
            }
        }
    }
}
