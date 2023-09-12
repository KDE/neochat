// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0
import './RoomList' as RoomList

QQC2.Dialog {
    id: root

    required property NeoChatConnection connection

    parent: applicationWindow().overlay
    width: Math.min(700, parent.width)
    height: 400

    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0
    topPadding: 0

    anchors.centerIn: applicationWindow().overlay


    Shortcut {
        sequence: "Ctrl+K"
        onActivated: root.open()
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }
        searchField.forceActiveFocus()
        searchField.text = ""
        roomList.currentIndex = 0
    }

    header: Kirigami.SearchField {
        id: searchField
        Keys.onDownPressed: {
            roomList.forceActiveFocus()
            if (roomList.currentIndex < roomList.count - 1) {
                roomList.currentIndex++
            } else {
                roomList.currentIndex = 0
            }
        }
        Keys.onUpPressed: {
            if (roomList.currentIndex === 0) {
                roomList.currentIndex = roomList.count - 1
            } else {
                roomList.currentIndex--
            }
        }
        Keys.onEnterPressed: {
            RoomManager.enterRoom(roomList.currentItem.currentRoom);
            root.close();
        }
        Keys.onReturnPressed: {
            RoomManager.enterRoom(roomList.currentItem.currentRoom);
            root.close();
        }
        focusSequence: ""
    }

    QQC2.ScrollView {
        anchors.fill: parent

        Keys.forwardTo: searchField

        ListView {
            id: roomList

            currentIndex: 0
            highlightMoveDuration: 200
            Keys.forwardTo: searchField
            keyNavigationEnabled: true
            model: SortFilterRoomListModel {
                filterText: searchField.text
                sourceModel: RoomListModel {
                    id: roomListModel
                    connection: root.connection
                }
            }

            delegate: RoomList.RoomDelegate {
                filterText: searchField.text

                connection: root.connection

                onClicked: {
                    RoomManager.enterRoom(currentRoom);
                    root.close()
                }

                Keys.onEnterPressed: {
                    RoomManager.enterRoom(currentRoom);
                    root.close();
                }

                Keys.onReturnPressed: {
                    RoomManager.enterRoom(currentRoom);
                    root.close();
                }
            }
        }
    }
}
