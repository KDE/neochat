import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.2
import QtQml.Models 2.3
import Matrique 0.1
import SortFilterProxyModel 0.2

import "qrc:/qml/component"

Item {
    property alias listModel: roomListProxyModel.sourceModel
    readonly property int currentIndex: roomListProxyModel.mapToSource(listView.currentIndex)
    readonly property var currentRoom: currentIndex != -1 ? listModel.roomAt(currentIndex) : null
    readonly property bool mini: setting.miniMode // Used as an indicator of whether the listform should be displayed as "Mini mode".
    signal enterRoom()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            z: 10
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Qt.tint(Material.accent, "#20FFFFFF")

            TextField {
                id: searchField
                width: parent.width - 18
                height: 36
                color: "white"
                leftPadding: mini ? 4 : 32
                topPadding: 0
                bottomPadding: 0
                anchors.centerIn: parent

                background: Row {
                    visible: !parent.text

                    MaterialIcon {
                        icon: "\ue8b6"
                        color: "white"

                        width: mini ? parent.width : parent.height
                        height: parent.height
                    }

                    Label {
                        height: parent.height
                        visible: !mini
                        text: "Search"
                        color: "white"
                        font.pointSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Material.theme == Material.Light ? "#eaeaea" : "#242424"

            Label {
                z: 10
                text: mini ? "Empty" : "Here? No, not here."
                anchors.centerIn: parent
                visible: listView.count === 0
            }

            SortFilterProxyModel {
                id: roomListProxyModel
                filters: RegExpFilter {
                    roleName: "name"
                    pattern: searchField.text
                    caseSensitivity: Qt.CaseInsensitive
                }
                proxyRoles: ExpressionRole {
                    name: "display"
                    expression: {
                        switch (category) {
                        case 1: return "Invited"
                        case 2: return "Favorites"
                        case 3: return "Rooms"
                        case 4: return "People"
                        case 5: return "Low Priorities"
                        }
                    }
                }

                sorters: [
                    RoleSorter { roleName: "category" },
                    StringSorter { roleName: "name" }
                ]
            }

            ListView {
                id: listView
                anchors.fill: parent

                model: roomListProxyModel

                highlight: Rectangle {
                    color: Material.accent
                    opacity: 0.2
                }
                highlightMoveDuration: 250
                highlightResizeDuration: 0

                currentIndex: -1

                boundsBehavior: Flickable.DragOverBounds

                ScrollBar.vertical: ScrollBar { id: scrollBar }

                delegate: ItemDelegate {
                    width: parent.width
                    height: 80
                    onPressed: listView.currentIndex = index
                    onPressAndHold: roomListMenu.popup()
                    onClicked: category === RoomType.Invited ? inviteDialog.open() : enterRoom()

                    ToolTip.visible: mini && hovered
                    ToolTip.text: name

                    Rectangle {
                        width: 4
                        height: parent.height
                        color: Qt.tint(Material.accent, "#20FFFFFF")
                        visible: unreadCount > 0
                    }

                    contentItem: RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        ImageStatus {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true

                            source: avatar ? "image://mxc/" + avatar : ""
                            displayText: name
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignHCenter

                            visible: parent.width > 80

                            Label {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                text: name ? name : ""
                                font.pointSize: 16
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                text: topic ? topic : "No topic yet."
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                        }
                    }
                }

                section.property: "display"
                section.criteria: ViewSection.FullString
                section.delegate: Label {
                    width: parent.width
                    height: 24
                    text: section
                    color: "grey"
                    leftPadding: mini ? undefined : 16
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: mini ? Text.AlignHCenter : undefined
                    background: Rectangle {
                        anchors.fill: parent
                        color: Material.theme == Material.Light ? "#dbdbdb" : "#363636"
                    }
                }

                Dialog {
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
                    onRejected: currentRoom.rejectInvitation()
                }

                Menu {
                    id: roomListMenu

                    MenuItem {
                        text: "Favourite"
                        checkable: true
                        checked: currentRoom && currentRoom.isFavourite
                        onTriggered: currentRoom.isFavourite ? currentRoom.removeTag("m.favourite") : currentRoom.addTag("m.favourite", "1")
                    }
                    MenuItem {
                        text: "Deprioritize"
                        checkable: true
                        checked: currentRoom && currentRoom.isLowPriority
                        onTriggered: currentRoom.isLowPriority ? currentRoom.removeTag("m.lowpriority") : currentRoom.addTag("m.lowpriority", "1")
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: "Mark as Read"
                        onTriggered: currentRoom.markAllMessagesAsRead()
                    }
                    MenuItem {
                        text: "Leave Room"
                        onTriggered: currentRoom.forget()
                    }
                }
            }
        }
    }

    onCurrentRoomChanged: setting.lazyLoad && currentRoom && !currentRoom.timelineSize ? currentRoom.getPreviousContent(20) : {}
}
