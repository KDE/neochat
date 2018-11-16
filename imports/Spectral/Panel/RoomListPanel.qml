import QtQuick 2.9
import QtQuick.Controls 2.2

import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral.Component 2.0
import Spectral.Menu 2.0

import Spectral 0.1
import Spectral.Setting 0.1

import SortFilterProxyModel 0.2

Rectangle {
    property var controller: null
    readonly property var user: controller.connection ? controller.connection.localUser : null

    readonly property int filter: 0
    property var enteredRoom: null
    property alias errorControl: errorControl

    signal enterRoom(var room)
    signal leaveRoom(var room)

    id: root

    color: MSettings.darkTheme ? "#303030" : "#FFFFFF"

    RoomListModel {
        id: roomListModel

        connection: controller.connection

        onNewMessage: if (!window.active) spectralController.postNotification(roomId, eventId, roomName, senderName, text, icon, iconPath)
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
                case 3: return "Rooms"
                case 4: return "People"
                case 5: return "Low Priority"
                }
            }
        }

        sorters: [
            RoleSorter { roleName: "category" },
            RoleSorter {
                roleName: "lastActiveTime"
                sortOrder: Qt.DescendingOrder
            }
        ]

        filters: [
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
                expression: category === 1 || category === 2 || category === 4
            },
            ExpressionFilter {
                enabled: filter === 3
                expression: category === 3 || category === 5
            }
        ]
    }

    Drawer {
        width: Math.max(root.width, 360)
        height: root.height

        id: drawer

        edge: Qt.LeftEdge

        ColumnLayout {
            width: parent.width
            spacing: 0

            Control {
                Layout.fillWidth: true
                Layout.preferredHeight: 330

                padding: 24

                contentItem: ColumnLayout {
                    spacing: 4

                    ImageItem {
                        Layout.preferredWidth: 200
                        Layout.preferredHeight: 200
                        Layout.margins: 12
                        Layout.alignment: Qt.AlignHCenter

                        source: root.user ? root.user.paintable : null
                        hint: root.user ? root.user.displayName : "?"
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter

                        text: root.user ? root.user.displayName : "No Name"
                        color: "white"
                        font.pointSize: 16.5
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter

                        text: root.user ? root.user.id : "@example:matrix.org"
                        color: "white"
                        opacity: 0.7
                        font.pointSize: 9.75
                    }
                }

                background: Rectangle { color: "#455A64" }
            }

            Repeater {
                model: AccountListModel {
                    controller: spectralController
                }

                delegate: ItemDelegate {
                    Layout.fillWidth: true

                    text: user.displayName

                    onClicked: controller.connection = connection
                }
            }

            ItemDelegate {
                Layout.fillWidth: true

                text: "Exit"

                onClicked: Qt.quit()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Control {
            Layout.fillWidth: true
            Layout.preferredHeight: 64

            topPadding: 12
            bottomPadding: 12
            leftPadding: 12
            rightPadding: 18

            contentItem: RowLayout {
                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    contentItem: MaterialIcon {
                        icon: searchField.visible ? "\ue5cd" : "\ue8b6"
                        color: searchField.visible ? "#1D333E" : "#7F7F7F"
                    }

                    onClicked: {
                        if (searchField.visible) searchField.clear()
                        searchField.visible = !searchField.visible
                    }
                }

                AutoTextField {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: searchField

                    visible: false
                    topPadding: 0
                    bottomPadding: 0
                    placeholderText: "Search..."

                    background: Item {}
                }

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    visible: !searchField.visible

                    text: root.user ? root.user.displayName : "No Name"
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    font.pointSize: 12
                    color: "#7F7F7F"
                    verticalAlignment: Text.AlignVCenter
                }

                ImageItem {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    visible: !searchField.visible

                    source: root.user ? root.user.paintable : null
                    hint: root.user ? root.user.displayName : "?"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: drawer.open()
                    }
                }
            }
        }

        Control {
            property string error: ""
            property string detail: ""

            Layout.fillWidth: true

            id: errorControl

            visible: false

            topPadding: 16
            bottomPadding: 16
            leftPadding: 24
            rightPadding: 24

            contentItem: ColumnLayout {
                Label {
                    Layout.fillWidth: true

                    text: errorControl.error
                    font.pointSize: 12
                    color: "white"
                    wrapMode: Text.Wrap
                }
                Label {
                    Layout.fillWidth: true

                    text: errorControl.detail
                    font.pointSize: 10.5
                    color: "white"
                    opacity: 0.6
                    wrapMode: Text.Wrap
                }
                ItemDelegate {
                    Layout.preferredHeight: 32
                    Layout.alignment: Qt.AlignRight

                    text: "Dismiss"
                    Material.foreground: "white"
                    onClicked: errorControl.visible = false
                }
            }

            background: Rectangle { color: "#273338" }
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

            delegate: RoomListDelegate {
                width: parent.width
                height: 64
            }

            section.property: "display"
            section.criteria: ViewSection.FullString
            section.delegate: Label {
                width: parent.width
                height: 24

                text: section
                color: "#5B7480"
                leftPadding: 16
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            RoomContextMenu {
                id: roomContextMenu
            }
        }
    }

    Dialog {
        property var currentRoom

        id: inviteDialog
        parent: ApplicationWindow.overlay

        x: (window.width - width) / 2
        y: (window.height - height) / 2
        width: 360

        title: "Action Required"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: Label { text: "Accept this invitation?" }

        onAccepted: currentRoom.acceptInvitation()
        onRejected: currentRoom.forget()
    }
}
