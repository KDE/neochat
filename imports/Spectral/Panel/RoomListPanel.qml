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
                enabled: filter === 1
                expression: unreadCount > 0
            },
            ExpressionFilter {
                enabled: filter === 2
                expression: category === 1 || category === 2 || category === 3
            },
            ExpressionFilter {
                enabled: filter === 3
                expression: category === 4 || category === 5
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
                        icon: {
                            switch (filter) {
                            case 0: return "\ue8b6"
                            case 1: return "\ue7f5"
                            case 2: return "\ue7ff"
                            case 3: return "\ue7fc"
                            }
                        }
                    }

                    Menu {
                        id: filterMenu

                        MenuItem {
                            text: "All"

                            onClicked: filter = 0
                        }

                        MenuSeparator {}

                        MenuItem {
                            text: "New"

                            onClicked: filter = 1
                        }

                        MenuItem {
                            text: "People"

                            onClicked: filter = 2
                        }

                        MenuItem {
                            text: "Group"

                            onClicked: filter = 3
                        }
                    }

                    onClicked: filterMenu.popup()
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

                opacity: listView.atYBeginning ? 0 : 1

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

            spacing: 0
            clip: true

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
                        color: "white"
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

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: {
                        if (category === RoomType.Invited) {
                            acceptInvitationDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom}).open()
                        } else {
                            if (enteredRoom) {
                                leaveRoom(enteredRoom)
                                enteredRoom.displayed = false
                            }
                            enterRoom(currentRoom)
                            enteredRoom = currentRoom
                            currentRoom.displayed = true
                        }
                    }
                    onSecondaryClicked: roomListContextMenu.createObject(ApplicationWindow.overlay, {"room": currentRoom}).popup()
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
        }
    }

    Component {
        id: acceptInvitationDialog

        AcceptInvitationDialog {}
    }
}
