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

    Label {
        z: 10
        text: MSettings.miniMode ? "Empty" : "Here? No, not here."
        anchors.centerIn: parent
        visible: listView.count === 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 12
            leftPadding: MSettings.miniMode ? 4 : 32
            topPadding: 0
            bottomPadding: 0
            placeholderText: "Search..."

            background: Rectangle {
                color: MSettings.darkTheme ? "#242424" : "#fafafa"
            }

            Shortcut {
                sequence: StandardKey.Find
                onActivated: searchField.forceActiveFocus()
            }
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
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: listView

            spacing: 1
            clip: true

            model: roomListProxyModel

            currentIndex: -1

            boundsBehavior: Flickable.DragOverBounds

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                readonly property bool highlighted: currentRoom === enteredRoom

                width: parent.width
                height: 64

                color: "#fafafa"

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
                    color: Material.accent
                    visible: unreadCount > 0 || highlighted
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

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

                        visible: parent.width > 64

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: name || "No Name"
                            font.pointSize: 12
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: (lastEvent || topic).replace(/(\r\n\t|\n|\r\t)/gm,"");
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
