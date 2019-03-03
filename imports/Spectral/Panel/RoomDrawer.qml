import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0

import Spectral 0.1

Drawer {
    property var room

    id: roomDrawer

    edge: Qt.RightEdge

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32

        Avatar {
            Layout.preferredWidth: 96
            Layout.preferredHeight: 96
            Layout.alignment: Qt.AlignHCenter

            hint: room ? room.displayName : "No name"
            source: room ? room.avatarMediaId : null
        }

        Label {
            Layout.fillWidth: true

            wrapMode: Label.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: room && room.id ? room.id : ""
        }

        Label {
            Layout.fillWidth: true

            wrapMode: Label.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: room && room.canonicalAlias ? room.canonicalAlias : "No Canonical Alias"
        }

        Label {
            Layout.fillWidth: true

            wrapMode: Label.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: room ? room.totalMemberCount + " Members" : "No Member Count"
        }

        RowLayout {
            Layout.fillWidth: true

            AutoTextField {
                Layout.fillWidth: true

                id: roomNameField
                text: room && room.name ? room.name : ""
            }

            ItemDelegate {
                Layout.preferredWidth: height
                Layout.preferredHeight: parent.height

                contentItem: MaterialIcon { icon: "\ue5ca" }

                onClicked: room.setName(roomNameField.text)
            }
        }

        RowLayout {
            Layout.fillWidth: true

            AutoTextField {
                Layout.fillWidth: true

                id: roomTopicField

                text: room && room.topic ? room.topic : ""
            }

            ItemDelegate {
                Layout.preferredWidth: height
                Layout.preferredHeight: parent.height

                contentItem: MaterialIcon { icon: "\ue5ca" }

                onClicked: room.setTopic(roomTopicField.text)
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

        Button {
            Layout.fillWidth: true

            text: "Invite User"
            flat: true
            highlighted: true

            onClicked: inviteUserDialog.open()

            Dialog {
                x: (window.width - width) / 2
                y: (window.height - height) / 2
                width: 360

                id: inviteUserDialog

                parent: ApplicationWindow.overlay

                title: "Input User ID"
                modal: true
                standardButtons: Dialog.Ok | Dialog.Cancel

                contentItem: AutoTextField {
                    id: inviteUserDialogTextField
                    placeholderText: "@bot:matrix.org"
                }

                onAccepted: room.inviteToRoom(inviteUserDialogTextField.text)
            }
        }
    }
}
