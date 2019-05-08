import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

import Spectral 0.1

Drawer {
    property var room

    id: roomDrawer

    edge: Qt.RightEdge

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24

        Component {
            id: fullScreenImage

            FullScreenImage {}
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72

                hint: room ? room.displayName : "No name"
                source: room ? room.avatarMediaId : null

                RippleEffect {
                    anchors.fill: parent

                    circular: true

                    onClicked: fullScreenImage.createObject(parent, {"filename": room.diaplayName, "localPath": room.urlToMxcUrl(room.avatarUrl)}).show()
                }
            }

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    font.pixelSize: 18
                    font.bold: true
                    wrapMode: Label.Wrap
                    text: room ? room.displayName : "No Name"
                    color: MPalette.foreground
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

        Control {
            Layout.fillWidth: true

            padding: 0

            contentItem: RowLayout {
                spacing: 8

                MaterialIcon {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    Layout.alignment: Qt.AlignTop

                    icon: "\ue88f"
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
                        color: MPalette.foreground
                    }

                    Label {
                        Layout.fillWidth: true

                        wrapMode: Label.Wrap
                        text: "Topic"
                        color: MPalette.lighter
                    }
                }
            }

            background: RippleEffect {
                onPrimaryClicked: roomSettingDialog.createObject(ApplicationWindow.overlay, {"room": room}).open()
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

                onClicked: inviteUserDialog.createObject(ApplicationWindow.overlay, {"room": room}).open()
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

            delegate: Item {
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
                        color: MPalette.foreground
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": room, "user": user}).open()
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

    Component {
        id: roomSettingDialog

        RoomSettingsDialog {}
    }

    Component {
        id: userDetailDialog

        UserDetailDialog {}
    }

    Component {
        id: inviteUserDialog

        InviteUserDialog {}
    }
}
