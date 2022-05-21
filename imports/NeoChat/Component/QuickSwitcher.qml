// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 1.0

QQC2.Popup {
    id: _popup

    Shortcut {
        sequence: "Ctrl+K"
        enabled: !Kirigami.Settings.hasPlatformMenuBar
        onActivated: _popup.open()
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }
        quickSearch.forceActiveFocus()
        quickSearch.text = ""
    }

    anchors.centerIn: QQC2.Overlay.overlay
    background: Kirigami.Card {}
    height: 2 * Math.round(implicitHeight / 2)
    padding: Kirigami.Units.largeSpacing * 2

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing * 2

        Kirigami.SearchField {
            id: quickSearch

            // TODO: get this broken property removed/disabled by default in Kirigami,
            // we used to be able to expect that the text field wouldn't attempt to
            // perform a mini-DDOS attack using signals.
            autoAccept: false

            Layout.preferredWidth: Kirigami.Units.gridUnit * 21 // 3 * 7 = 21, roughly 7 avatars on screen
            Keys.onLeftPressed: cView.decrementCurrentIndex()
            Keys.onRightPressed: cView.incrementCurrentIndex()
            onAccepted: {
                const item = cView.itemAtIndex(cView.currentIndex)

                RoomManager.enterRoom(item.currentRoom)

                _popup.close()
            }
        }
        ListView {
            id: cView

            orientation: Qt.Horizontal
            spacing: Kirigami.Units.largeSpacing

            model: SortFilterRoomListModel {
                id: sortFilterRoomListModel
                sourceModel: RoomListModel {
                    id: roomListModel
                    connection: Controller.activeConnection
                }
                filterText: quickSearch.text
                roomSortOrder: SortFilterRoomListModel.LastActivity
            }

            Layout.preferredHeight: Kirigami.Units.gridUnit * 3
            Layout.fillWidth: true

            delegate: Kirigami.Avatar {
                id: del

                implicitHeight: Kirigami.Units.gridUnit * 3
                implicitWidth: Kirigami.Units.gridUnit * 3

                required property string avatar
                required property var currentRoom
                required property int index

                // When an item is hovered set the currentIndex of listview to it so that it is highlighted
                onHoveredChanged: {
                    if (!hovered) {
                        return
                    }
                    cView.currentIndex = index
                }

                actions.main: Kirigami.Action {
                    id: enterRoomAction
                    onTriggered: {
                        RoomManager.enterRoom(currentRoom);

                        _popup.close()
                    }
                }

                source: avatar != "" ? "image://mxc/" + avatar : ""
            }
        }
    }

    modal: true
    focus: true
}
