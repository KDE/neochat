import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

import Spectral.Component 2.0

import Spectral 0.1

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
            Layout.preferredWidth: 96
            Layout.preferredHeight: 96
            Layout.alignment: Qt.AlignHCenter

            hint: room ? room.displayName : "No name"
            source: room ? room.paintable : null
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
            text: room ? room.memberCount + " Members" : "No Member Count"
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

            delegate: Column {
                property bool expanded: false

                ItemDelegate {
                    width: userListView.width
                    height: 48

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 12

                        ImageItem {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true

                            source: paintable
                            hint: name
                        }

                        Label {
                            Layout.fillWidth: true

                            text: name
                        }
                    }

                    onClicked: expanded = !expanded
                }

                ColumnLayout {
                    width: parent.width - 32
                    height: expanded ? implicitHeight : 0
                    anchors.horizontalCenter: parent.horizontalCenter

                    spacing: 0

                    clip: true

                    Button {
                        Layout.fillWidth: true

                        text: "Kick"
                        highlighted: true

                        onClicked: room.kickMember(userId)
                    }

                    Behavior on height {
                        PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
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

                contentItem: AutoTextField {
                    id: inviteUserDialogTextField
                    placeholderText: "@bot:matrix.org"
                }

                onAccepted: room.inviteToRoom(inviteUserDialogTextField.text)
            }
        }
    }
}
