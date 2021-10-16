// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0


Kirigami.OverlayDrawer {
    id: roomDrawer
    readonly property var room: RoomManager.currentRoom

    enabled: true

    edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    contentItem: Loader {
        active: roomDrawer.drawerOpen
        sourceComponent: ColumnLayout {
            id: columnLayout
            anchors.fill: parent
            spacing: 0
            Kirigami.AbstractApplicationHeader {
                Layout.fillWidth: true
                topPadding: Kirigami.Units.smallSpacing / 2;
                bottomPadding: Kirigami.Units.smallSpacing / 2;
                rightPadding: Kirigami.Units.smallSpacing
                leftPadding: Kirigami.Units.smallSpacing

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    ToolButton {
                        icon.name: "list-add-user"
                        text: i18n("Invite")
                        onClicked: {
                            applicationWindow().pageStack.layers.push("qrc:/imports/NeoChat/Page/InviteUserPage.qml", {"room": room})
                            roomDrawer.close();
                        }
                    }
                    Item {
                        // HACK otherwise rating item is not right aligned
                        Layout.fillWidth: true
                    }

                    ToolButton {
                        Layout.alignment: Qt.AlignRight
                        icon.name: room && room.isFavourite ? "rating" : "rating-unrated"
                        checkable: true
                        checked: room && room.isFavourite
                        onClicked: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
                        ToolTip {
                            text: room && room.isFavourite ? i18n("Remove room from favorites") : i18n("Make room favorite")
                        }
                    }
                    ToolButton {
                        Layout.alignment: Qt.AlignRight
                        icon.name: 'settings-configure'
                        onClicked: {
                            roomSettingDialog.createObject(ApplicationWindow.overlay, {"room": room}).open()
                            if (!wideScreen) {
                                roomDrawer.close();
                            }
                        }

                        ToolTip {
                            text: i18n("Room settings")
                        }
                    }
                }
            }

            Control {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing
                contentItem: ColumnLayout {
                    id: infoLayout
                    Layout.fillWidth: true
                    Kirigami.Heading {
                        text: i18n("Room information")
                        level: 3
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.largeSpacing

                        spacing: Kirigami.Units.largeSpacing

                        Kirigami.Avatar {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 3.5
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 3.5

                            name: room ? room.name : i18n("No name")
                            source: room ? ("image://mxc/" +  room.avatarMediaId) : ""
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 0

                            Kirigami.Heading {
                                Layout.maximumWidth: Kirigami.Units.gridUnit * 9
                                Layout.fillWidth: true
                                level: 1
                                font.bold: true
                                wrapMode: Label.Wrap
                                text: room ? room.displayName : i18n("No name")
                            }
                            Label {
                                Layout.fillWidth: true
                                text: room && room.canonicalAlias ? room.canonicalAlias : i18n("No Canonical Alias")
                            }
                        }
                    }

                    TextEdit {
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 13
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 13
                        Layout.fillWidth: true
                        text: room && room.topic ? room.topic.replace(replaceLinks, "<a href=\"$1\">$1</a>") : i18n("No Topic")
                        readonly property var replaceLinks: /\(https:\/\/[^ ]*\)/
                        textFormat: TextEdit.MarkdownText
                        wrapMode: Text.WordWrap
                        selectByMouse: true
                        color: Kirigami.Theme.textColor
                        onLinkActivated: Qt.openUrlExternally(link)
                        readOnly: true
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                        }
                    }
                }
            }

            Kirigami.ListSectionHeader {
                label: i18n("Members")
                activeFocusOnTab: false
                Label {
                    Layout.alignment: Qt.AlignRight
                    text: room ? i18np("%1 Member", "%1 Members", room.joinedCount) : i18n("No Member Count")
                }
            }

            Pane {
                padding: Kirigami.Units.smallSpacing
                implicitWidth: parent.width
                z: 2
                contentItem: Kirigami.SearchField {
                    id: userListSearchField
                    onAccepted: sortedMessageEventModel.filterString = text;
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
                    activeFocusOnTab: true

                    model: KSortFilterProxyModel {
                        id: sortedMessageEventModel

                        sourceModel: UserListModel {
                            room: roomDrawer.room
                        }

                        sortRole: "perm"
                        filterRole: "name"
                        filterCaseSensitivity: Qt.CaseInsensitive
                    }

                    delegate: Kirigami.AbstractListItem {
                        width: userListView.width
                        implicitHeight: Kirigami.Units.gridUnit * 2
                        z: 1

                        contentItem: RowLayout {
                            Kirigami.Avatar {
                                Layout.preferredWidth: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                Layout.preferredHeight: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                visible: Config.showAvatarInRoomDrawer
                                sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                source: avatar ? ("image://mxc/" + avatar) : ""
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
                                color: Kirigami.Theme.disabledTextColor
                                font.pixelSize: 12
                                textFormat: Text.PlainText
                                wrapMode: Text.NoWrap
                            }
                        }

                        action: Kirigami.Action {
                            onTriggered: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": room, "user": user, "displayName": name, "avatarMediaId": avatar}).open()
                        }
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
