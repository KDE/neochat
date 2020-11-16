/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Effect 1.0
import NeoChat.Setting 1.0

import org.kde.neochat 1.0

Kirigami.OverlayDrawer {
    id: roomDrawer
    property var room

    enabled: true

    edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    width: 400
    implicitWidth: 400
    contentItem: ColumnLayout {
        implicitWidth: Kirigami.Units.gridUnit * 20

        Component {
            id: fullScreenImage

            FullScreenImage {}
        }

        ToolBar {
            Layout.fillWidth: true
            implicitHeight: infoLayout.implicitHeight
            bottomPadding: Kirigami.Units.largeSpacing
            contentItem: ColumnLayout {
                id: infoLayout
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing

                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Avatar {
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3.5
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 3.5

                        name: room ? room.displayName : i18n("No name")
                        source: room ? "image://mxc/" +  room.avatarMediaId : undefined
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Kirigami.Heading {
                            Layout.fillWidth: true
                            level: 1
                            font.bold: true
                            wrapMode: Label.Wrap
                            text: room ? room.displayName : i18n("No name")
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                Layout.fillWidth: true

                                wrapMode: Label.Wrap
                                text: room ? i18np("%1 Member", "%1 Members", room.totalMemberCount) : i18n("No Member Count")
                                color: Kirigami.Theme.disabledTextColor
                            }
                            Button {
                                icon.name: 'settings-configure'
                                text: i18n("Room setting")
                                onClicked: {
                                    roomSettingDialog.createObject(ApplicationWindow.overlay, {"room": room}).open()
                                    roomDrawer.close();
                                }
                            }
                        }
                    }
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }

                Kirigami.FormLayout {
                    Layout.fillWidth: true
                    Label {
                        Kirigami.FormData.label: i18n("Main Alias")
                        text: room && room.canonicalAlias ? room.canonicalAlias :i18n("No Canonical Alias")
                    }
                    Label {
                        Kirigami.FormData.label: i18n("Topic")
                        text: room && room.topic ? room.topic : i18n("No Topic")
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 8
                    Kirigami.Icon {
                        source: "user-others"
                    }

                    Label {
                        Layout.fillWidth: true

                        wrapMode: Label.Wrap
                        text: room ?  i18np("%1 Member", "%1 Members", room.totalMemberCount) : i18n("No Member Count")
                    }

                    ToolButton {
                        icon.name: "list-add-user"
                        text: i18n("Invite")
                        onClicked: {
                            applicationWindow().pageStack.push("qrc:/imports/NeoChat/Page/InviteUserPage.qml", {"room": room})
                            roomDrawer.close();
                        }
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: userListView
                clip: true
                headerPositioning: ListView.OverlayHeader
                boundsBehavior: Flickable.DragOverBounds

                model: UserListModel {
                    room: roomDrawer.room
                }

                delegate: Kirigami.AbstractListItem {
                    width: userListView.width
                    implicitHeight: Kirigami.Units.gridUnit * 2

                    contentItem: RowLayout {
                        Kirigami.Avatar {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true

                            source: "image://mxc/" + avatar
                            name: name
                        }

                        Label {
                            Layout.fillWidth: true

                            text: name
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            visible: perm != UserType.Member

                            text: {
                                if (perm == UserType.Owner) {
                                    return i18n("Owner")
                                }
                                if (perm == UserType.Admin) {
                                    return i18n("Admin")
                                }
                                if (perm == UserType.Moderator) {
                                    return i18n("Mod")
                                }
                                if (perm == UserType.Muted) {
                                    return i18n("Muted")
                                }
                                return ""
                            }
                            color: perm == UserType.Muted ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                            font.pixelSize: 12
                            textFormat: Text.PlainText
                            wrapMode: Text.NoWrap
                        }
                    }

                    action: Kirigami.Action {
                        onTriggered: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": room, "user": user}).open()
                    }
                }
            }
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
}
