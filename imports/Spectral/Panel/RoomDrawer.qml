import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.13 as Kirigami

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

import Spectral 0.1

Kirigami.OverlayDrawer {
    property var room

    id: roomDrawer
    enabled: true

    edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge

    padding: 0
    contentItem: ColumnLayout {
        implicitWidth: Kirigami.Units.gridUnit * 15 // TODO FIXME

        Component {
            id: fullScreenImage

            FullScreenImage {}
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Kirigami.Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72

                name: room ? room.displayName : "No name"
                source: room ? "image://mxc/" +  room.avatarMediaId : null
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

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Control {
            Layout.fillWidth: true
            contentItem: Kirigami.FormLayout {
                Label {
                    Kirigami.FormData.label: "Main Alias"
                    text: room && room.canonicalAlias ? room.canonicalAlias : "No Canonical Alias"
                }
                Label {
                    Kirigami.FormData.label: "Topic"
                    text: room && room.topic ? room.topic : "No Topic"
                    wrapMode: Text.WordWrap
                }
            }

            background: AutoMouseArea {
                onPrimaryClicked: roomSettingDialog.createObject(ApplicationWindow.overlay, {"room": room}).open()
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 8
            Kirigami.Icon {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                source: "user-others"
            }

            Label {
                Layout.fillWidth: true

                wrapMode: Label.Wrap
                text: room ? room.totalMemberCount + " Members" : "No Member Count"
            }

            ToolButton {
                Layout.preferredWidth: Kirigami.Units.gridUnit
                Layout.preferredHeight: Kirigami.Units.gridUnit
                icon.name: "list-user-add"

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

                    Kirigami.Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: "image://mxc/" + avatar
                        name: name
                    }

                    Label {
                        Layout.fillWidth: true

                        text: name
                        color: MPalette.foreground
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        visible: perm != UserType.Member

                        text: {
                            if (perm == UserType.Owner) {
                                return "Owner"
                            }
                            if (perm == UserType.Admin) {
                                return "Admin"
                            }
                            if (perm == UserType.Moderator) {
                                return "Mod"
                            }
                            if (perm == UserType.Muted) {
                                return "Muted"
                            }
                            return ""
                        }
                        color: perm == UserType.Muted ? MPalette.lighter : MPalette.accent
                        font.pixelSize: 12
                        textFormat: Text.PlainText
                        wrapMode: Text.NoWrap
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

    onRoomChanged: {
        if (room == null) {
            close()
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
