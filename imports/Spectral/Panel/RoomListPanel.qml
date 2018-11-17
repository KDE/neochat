import QtQuick 2.9
import QtQuick.Controls 2.2

import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral.Component 2.0
import Spectral.Menu 2.0
import Spectral.Effect 2.0

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

        Component {
            id: mainPage

            ColumnLayout {
                readonly property string title: "Main"

                id: mainColumn

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

                    background: Rectangle { color: Material.primary }

                    ItemDelegate {
                        anchors.fill: parent
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ColumnLayout {
                        width: mainColumn.width
                        spacing: 0

                        Repeater {
                            model: AccountListModel {
                                controller: spectralController
                            }

                            delegate: ItemDelegate {
                                Layout.fillWidth: true

                                text: user.displayName

                                onClicked: {
                                    controller.connection = connection
                                    drawer.close()
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1

                            color: "#e7ebeb"
                        }

                        ItemDelegate {
                            Layout.fillWidth: true

                            text: "Settings"

                            onClicked: stackView.push(settingsPage)
                        }

                        ItemDelegate {
                            Layout.fillWidth: true

                            text: "Exit"

                            onClicked: Qt.quit()
                        }
                    }
                }
            }
        }

        Component {
            id: settingsPage

            ScrollView {
                readonly property string title: "Settings"

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                padding: 16

                ColumnLayout {
                    width: parent.width
                    spacing: 0

                    Switch {
                        text: "Dark theme"
                        checked: MSettings.darkTheme

                        onCheckedChanged: MSettings.darkTheme = checked
                    }

                    Switch {
                        text: "Use press and hold instead of right click"
                        checked: MSettings.pressAndHold

                        onCheckedChanged: MSettings.pressAndHold = checked
                    }

                    Switch {
                        text: "Show tray icon"
                        checked: MSettings.showTray

                        onCheckedChanged: MSettings.showTray = checked
                    }

                    Switch {
                        text: "Confirm on Exit"
                        checked: MSettings.confirmOnExit

                        onCheckedChanged: MSettings.confirmOnExit = checked
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            Control {
                Layout.fillWidth: true
                Layout.preferredHeight: 64

                visible: stackView.depth > 1

                contentItem: RowLayout {
                    anchors.fill: parent
                    ToolButton {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        contentItem: MaterialIcon {
                            icon: "\ue5c4"
                            color: "white"
                        }

                        onClicked: stackView.pop()
                    }
                    Label {
                        Layout.fillWidth: true

                        text: stackView.currentItem.title
                        color: "white"
                        font.pointSize: 13.5
                        elide: Label.ElideRight
                    }
                }

                background: Rectangle {
                    color: Material.primary
                }
            }

            StackView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: stackView

                initialItem: mainPage
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Control {
            readonly property bool isSearching: searchField.text

            Layout.fillWidth: true
            Layout.preferredHeight: 64

            id: roomListHeader

            topPadding: 12
            bottomPadding: 12
            leftPadding: 12
            rightPadding: 18

            contentItem: RowLayout {
                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    contentItem: MaterialIcon {
                        icon: roomListHeader.isSearching ? "\ue5cd" : "\ue8b6"
                        color: roomListHeader.isSearching ? "#1D333E" : "7F7F7F"
                    }

                    onClicked: {
                        if (searchField.focus) {
                            searchField.clear()
                            searchField.focus = false
                        } else {
                            searchField.focus = true
                        }
                    }
                }

                AutoTextField {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: searchField

                    topPadding: 0
                    bottomPadding: 0
                    placeholderText: "Search..."

                    background: Item {}
                }

                ImageItem {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignRight

                    visible: !roomListHeader.isSearching

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
            }

            background: Rectangle {
                color: "#273338"
            }

            ItemDelegate {
                anchors.fill: parent

                onClicked: errorControl.visible = false
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
