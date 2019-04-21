import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Setting 0.1

import Spectral 0.1

Drawer {
    property var room

    id: roomDrawer

    edge: Qt.RightEdge

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24

        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72

                hint: room ? room.displayName : "No name"
                source: room ? room.avatarMediaId : null
            }

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    font.pixelSize: 18
                    font.bold: true
                    wrapMode: Label.Wrap
                    text: room ? room.displayName : "No Name"
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: room ? room.totalMemberCount + " Members" : "No Member Count"
                    color: MPalette.lighter
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            MaterialIcon {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignTop

                icon: "\ue88e"
                color: MPalette.lighter
            }

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: room && room.canonicalAlias ? room.canonicalAlias : "No Canonical Alias"
                    color: MPalette.accent
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: "Main Alias"
                    color: MPalette.lighter
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: room && room.topic ? room.topic : "No Topic"
                    color: MPalette.accent
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: "Topic"
                    color: MPalette.lighter
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            MaterialIcon {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32

                icon: "\ue7ff"
                color: MPalette.lighter
            }

            Label {
                Layout.fillWidth: true

                wrapMode: Label.Wrap
                text: room ? room.totalMemberCount + " Members" : "No Member Count"
                color: MPalette.lighter
            }

            ToolButton {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32

                contentItem: MaterialIcon {
                    icon: "\ue145"
                    color: MPalette.lighter
                }

                onClicked: inviteUserDialog.open()
            }
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: userListView

            clip: true

            boundsBehavior: Flickable.DragOverBounds

            model: UserListModel {
                room: roomDrawer.room
            }

            delegate: SwipeDelegate {
                width: userListView.width
                height: 48

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 12

                    Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: avatar
                        hint: name
                    }

                    Label {
                        Layout.fillWidth: true

                        text: name
                    }
                }

                swipe.right: Rectangle {
                    width: height
                    height: parent.height
                    anchors.right: parent.right
                    color: Material.accent

                    MaterialIcon {
                        anchors.fill: parent
                        icon: "\ue879"
                        color: "white"
                    }

                    SwipeDelegate.onClicked: {
                        room.kickMember(userId)
                        swipe.close()
                    }
                }

                onClicked: swipe.open(SwipeDelegate.Right)
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

    Dialog {
        anchors.centerIn: parent
        width: 360

        id: inviteUserDialog

        parent: ApplicationWindow.overlay

        title: "Invite User"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: AutoTextField {
            id: inviteUserDialogTextField
            placeholderText: "@bot:matrix.org"
        }

        onAccepted: room.inviteToRoom(inviteUserDialogTextField.text)
    }
}
