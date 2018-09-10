import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1

import "qrc:/js/util.js" as Util

Drawer {
    property var room

    id: roomDrawer

    edge: Qt.RightEdge
    interactive: false

    ToolButton {
        contentItem: MaterialIcon { icon: "\ue5c4" }

        onClicked: roomDrawer.close()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32

        ImageItem {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            Layout.alignment: Qt.AlignHCenter

            hint: room ? room.displayName : "No name"
            defaultColor: Util.stringToColor(room ? room.displayName : "No name")
            image: matriqueController.safeImage(room ? room.avatar : null)
        }

        Label {
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            text: room && room.id ? room.id : ""
        }

        Label {
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            text: room && room.canonicalAlias ? room.canonicalAlias : "No Canonical Alias"
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                Layout.fillWidth: true

                id: roomNameField
                text: room && room.name ? room.name : ""
                selectByMouse: true
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

            TextField {
                Layout.fillWidth: true

                id: roomTopicField

                text: room && room.topic ? room.topic : ""
                selectByMouse: true
            }

            ItemDelegate {
                Layout.preferredWidth: height
                Layout.preferredHeight: parent.height

                contentItem: MaterialIcon { icon: "\ue5ca" }

                onClicked: room.setTopic(roomTopicField.text)
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            boundsBehavior: Flickable.DragOverBounds

            model: UserListModel {
                room: roomDrawer.room
            }

            delegate: ItemDelegate {
                width: parent.width
                height: 48

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 12

                    ImageItem {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        defaultColor: Util.stringToColor(name)
                        image: avatar
                        hint: name
                    }

                    Label {
                        Layout.fillWidth: true

                        text: name
                    }
                }
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

                contentItem: TextField {
                    id: inviteUserDialogTextField
                    placeholderText: "@bot:matrix.org"
                }

                onAccepted: room.inviteToRoom(inviteUserDialogTextField.text)
            }
        }
    }
}

