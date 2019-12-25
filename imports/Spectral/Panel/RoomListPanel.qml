import QtQuick 2.12
import QtQuick.Controls 2.12

import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Menu 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

import SortFilterProxyModel 0.2

Item {
    property var connection: null
    readonly property var user: connection ? connection.localUser : null

    property int filter: 0
    property var enteredRoom: null

    signal enterRoom(var room)
    signal leaveRoom(var room)

    id: root

    RoomListModel {
        id: roomListModel

        connection: root.connection

        onNewMessage: if (!window.active && MSettings.showNotification) notificationsManager.postNotification(roomId, eventId, roomName, senderName, text, icon)
    }

    Binding {
        target: trayIcon
        property: "notificationCount"
        value: roomListModel.notificationCount
    }

    SortFilterProxyModel {
        id: sortedRoomListModel

        sourceModel: roomListModel

        proxyRoles: ExpressionRole {
            name: "display"
            expression: {
                switch (category) {
                case 1: return "Invited"
                case 2: return "Favorites"
                case 3: return "People"
                case 4: return "Rooms"
                case 5: return "Low Priority"
                }
            }
        }

        sorters: [
            RoleSorter { roleName: "category" },
            ExpressionSorter {
                expression: {
                    return modelLeft.highlightCount > 0;
                }
            },
            ExpressionSorter {
                expression: {
                    return modelLeft.notificationCount > 0;
                }
            },
            RoleSorter {
                roleName: "lastActiveTime"
                sortOrder: Qt.DescendingOrder
            }
        ]

        filters: [
            ExpressionFilter {
                expression: joinState != "upgraded"
            },
            RegExpFilter {
                roleName: "name"
                pattern: searchField.text
                caseSensitivity: Qt.CaseInsensitive
            },
            ExpressionFilter {
                enabled: filter === 0
                expression: category !== 5 && notificationCount > 0 || currentRoom === enteredRoom
            },
            ExpressionFilter {
                enabled: filter === 1
                expression: category === 1 || category === 3
            },
            ExpressionFilter {
                enabled: filter === 2
                expression: category !== 3
            }
        ]
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: searchField.forceActiveFocus()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Control {
            Layout.fillWidth: true
            Layout.preferredHeight: 64

            id: roomListHeader

            topPadding: 12
            bottomPadding: 12
            leftPadding: 12
            rightPadding: 18

            contentItem: RowLayout {
                ToolButton {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    visible: !searchField.active

                    contentItem: MaterialIcon {
                        icon: "\ue8b6"
                    }
                }

                ToolButton {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    visible: searchField.active

                    contentItem: MaterialIcon { icon: "\ue5cd" }

                    onClicked: searchField.clear()
                }

                AutoTextField {
                    readonly property bool active: text

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    id: searchField

                    placeholderText: "Search..."
                    color: MPalette.lighter
                }

                Avatar {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignRight

                    visible: !searchField.active

                    source: root.user ? root.user.avatarMediaId : null
                    hint: root.user ? root.user.displayName : "?"

                    RippleEffect {
                        anchors.fill: parent

                        circular: true

                        onClicked: accountDetailDialog.createObject(ApplicationWindow.overlay).open()
                    }
                }
            }

            background: Rectangle {
                color: Material.background

                layer.enabled: true
                layer.effect: ElevationEffect {
                    elevation: 2
                }
            }
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: listView

            z: -1

            spacing: 0

            model: sortedRoomListModel

            boundsBehavior: Flickable.DragOverBounds

            ScrollBar.vertical: ScrollBar {}

            delegate: Item {
                width: listView.width
                height: 64

                Rectangle {
                    anchors.fill: parent

                    visible: currentRoom === enteredRoom
                    color: Material.accent
                    opacity: 0.1
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12

                    spacing: 12

                    Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: avatar
                        hint: name || "No Name"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: name || "No Name"
                            color: MPalette.foreground
                            font.pixelSize: 16
                            font.bold: unreadCount >= 0
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: (lastEvent == "" ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm," ")
                            color: MPalette.lighter
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                    }

                    Label {
                        visible: notificationCount > 0 && highlightCount == 0
                        color: MPalette.background
                        text: notificationCount
                        leftPadding: 12
                        rightPadding: 12
                        topPadding: 4
                        bottomPadding: 4
                        font.bold: true

                        background: Rectangle {
                            radius: height / 2
                            color: MPalette.lighter
                        }
                    }

                    Label {
                        visible: highlightCount > 0
                        color: "white"
                        text: highlightCount
                        leftPadding: 12
                        rightPadding: 12
                        topPadding: 4
                        bottomPadding: 4
                        font.bold: true

                        background: Rectangle {
                            radius: height / 2
                            color: MPalette.accent
                        }
                    }
                }

                AutoMouseArea {
                    anchors.fill: parent

                    onPrimaryClicked: {
                        if (category === RoomType.Invited) {
                            acceptInvitationDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom}).open()
                        } else {
                            joinRoom(currentRoom)
                        }
                    }
                    onSecondaryClicked: roomListContextMenu.createObject(parent, {"room": currentRoom}).popup()
                }

                Component {
                    id: roomListContextMenu

                    RoomListContextMenu {}
                }
            }

            section.property: "display"
            section.criteria: ViewSection.FullString
            section.delegate: Label {
                width: parent.width
                height: 24

                text: section
                color: MPalette.lighter
                leftPadding: 16
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            ColumnLayout {
                anchors.centerIn: parent

                visible: sortedRoomListModel.count == 0

                MaterialIcon {
                    Layout.alignment: Qt.AlignHCenter

                    icon: "\ue5ca"
                    font.pixelSize: 48
                    color: MPalette.lighter
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    text: "You're all caught up!"

                    color: MPalette.foreground
                }
            }
        }

        Control {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            Layout.margins: 16

            padding: 8

            contentItem: RowLayout {
                id: tabBar

                MaterialIcon {
                    Layout.fillWidth: true

                    icon: "\ue7f5"
                    color: filter == 0 ? MPalette.accent : MPalette.lighter

                    MouseArea {
                        anchors.fill: parent

                        onClicked: filter = 0
                    }
                }

                MaterialIcon {
                    Layout.fillWidth: true

                    icon: "\ue7ff"
                    color: filter == 1 ? MPalette.accent : MPalette.lighter

                    MouseArea {
                        anchors.fill: parent

                        onClicked: filter = 1
                    }
                }

                MaterialIcon {
                    Layout.fillWidth: true

                    icon: "\ue7fc"
                    color: filter == 2 ? MPalette.accent : MPalette.lighter

                    MouseArea {
                        anchors.fill: parent

                        onClicked: filter = 2
                    }
                }
            }

            background: AutoRectangle {
                color: MPalette.background

                radius: 24

                topLeftRadius: 8
                topRightRadius: 8

                topLeftVisible: true
                topRightVisible: true
                bottomLeftVisible: false
                bottomRightVisible: false

                layer.enabled: true
                layer.effect: ElevationEffect {
                    elevation: 1
                }
            }
        }
    }

    Component {
        id: acceptInvitationDialog

        AcceptInvitationDialog {}
    }

    function joinRoom(room) {
        if (enteredRoom) {
            leaveRoom(enteredRoom)
            enteredRoom.displayed = false
        }
        enterRoom(room)
        enteredRoom = room
        room.displayed = true
    }
}
