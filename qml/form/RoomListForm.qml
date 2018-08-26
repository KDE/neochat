import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.2
import QtQml.Models 2.3
import Matrique 0.1
import SortFilterProxyModel 0.2
import Matrique.Settings 0.1

import "../component"

Item {
    property alias listModel: roomListProxyModel.sourceModel
    property var enteredRoom: null

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
                leftPadding: MSettings.miniMode ? 4 : 32
                topPadding: 0
                bottomPadding: 0
                anchors.centerIn: parent

                background: Row {
                    visible: !parent.text

                    MaterialIcon {
                        icon: "\ue8b6"
                        color: "white"

                        width: MSettings.miniMode ? parent.width : parent.height
                        height: parent.height
                    }

                    Label {
                        height: parent.height
                        visible: !MSettings.miniMode
                        text: "Search"
                        color: "white"
                        font.pointSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Shortcut {
                    sequence: StandardKey.Find
                    onActivated: searchField.forceActiveFocus()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: MSettings.darkTheme ? "#242424" : "#eaeaea"

            Label {
                z: 10
                text: MSettings.miniMode ? "Empty" : "Here? No, not here."
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
                    RoleSorter {
                        enabled: MSettings.rearrangeByActivity
                        roleName: "unreadCount"
                        sortOrder: Qt.DescendingOrder
                    },
                    StringSorter { roleName: "name" }
                ]
            }

            ListView {
                id: listView
                anchors.fill: parent

                model: roomListProxyModel

                currentIndex: -1

                boundsBehavior: Flickable.DragOverBounds

                ScrollBar.vertical: ScrollBar { id: scrollBar }

                delegate: Rectangle {
                    readonly property bool highlighted: currentRoom === enteredRoom

                    id: swipeDelegate
                    width: parent.width
                    height: 80

                    color: highlighted ? Material.background : "transparent"

                    AutoMouseArea {
                        anchors.fill: parent

                        hoverEnabled: MSettings.miniMode

                        onSecondaryClicked: Qt.createComponent("qrc:/qml/menu/RoomContextMenu.qml").createObject(this)
                        onPrimaryClicked: category === RoomType.Invited ? inviteDialog.open() : enteredRoom = currentRoom

                        ToolTip.visible: MSettings.miniMode && containsMouse
                        ToolTip.text: name
                    }

                    Rectangle {
                        width: 4
                        height: parent.height
                        color: Qt.tint(Material.accent, "#20FFFFFF")
                        visible: unreadCount > 0 || highlighted
                    }

                    RowLayout {
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

                                text: name || "No Name"
                                font.pointSize: 16
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                text: lastEvent || topic
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
                    leftPadding: MSettings.miniMode ? undefined : 16
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: MSettings.miniMode ? Text.AlignHCenter : undefined
                    background: Rectangle {
                        anchors.fill: parent
                        color: MSettings.darkTheme ? "#363636" : "#dbdbdb"
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
                    onRejected: currentRoom.forget()
                }
            }
        }
    }
}
