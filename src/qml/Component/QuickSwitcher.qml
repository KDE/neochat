// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

QQC2.Dialog {
    id: root

    parent: applicationWindow().overlay
    width: Math.min(700, parent.width)
    height: 400

    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0
    topPadding: 0

    anchors.centerIn: applicationWindow().overlay

    Keys.forwardTo: searchField

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
    }

    QQC2.ScrollView {
        anchors.fill: parent
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
                    connection: Controller.activeConnection
                }
            }
            delegate: Kirigami.BasicListItem {
                id: roomListItem

                required property var currentRoom
                required property string name
                required property int index
                required property int unreadCount
                required property string subtitleText
                required property string avatar

                topPadding: Kirigami.Units.largeSpacing
                bottomPadding: Kirigami.Units.largeSpacing
                highlighted: roomList.currentIndex === roomListItem.index
                focus: true
                icon: undefined
                onClicked: {
                    RoomManager.enterRoom(roomListItem.currentRoom);
                    root.close()
                }
                Keys.onEnterPressed: {
                    RoomManager.enterRoom(roomListItem.currentRoom);
                    root.close();
                }
                Keys.onReturnPressed: {
                    RoomManager.enterRoom(roomListItem.currentRoom);
                    root.close();
                }
                bold: roomListItem.unreadCount > 0
                label: roomListItem.name ?? ""
                labelItem.textFormat: Text.PlainText
                subtitle: roomListItem.subtitleText
                subtitleItem.textFormat: Text.PlainText
                onPressAndHold: {
                    createRoomListContextMenu()
                }

                leading: Kirigami.Avatar {
                    source: roomListItem.avatar ? "image://mxc/" + roomListItem.avatar : ""
                    name: roomListItem.name || i18n("No Name")
                    implicitWidth: height
                    sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }
            }
        }
    }
}
