// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.neochat.settings

/**
 * Context menu when clicking on a room in the room list
 */
Loader {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow window

    signal closed

    Component {
        id: regularMenu
        QQC2.Menu {
            QQC2.MenuItem {
                text: i18nc("'Space' is a matrix space", "View Space")
                icon.name: "view-list-details"
                onTriggered: RoomManager.resolveResource(room.id)
            }

            QQC2.MenuItem {
                text: i18nc("@action:inmenu", "Copy Address to Clipboard")
                icon.name: "edit-copy"
                onTriggered: if (room.canonicalAlias.length === 0) {
                    Clipboard.saveText(room.id);
                } else {
                    Clipboard.saveText(room.canonicalAlias);
                }
            }

            QQC2.MenuItem {
                text: i18nc("'Space' is a matrix space", "Space Settings")
                icon.name: 'settings-configure-symbolic'
                onTriggered: {
                    RoomSettingsView.openRoomSettings(root.room, RoomSettingsView.Space);
                }
            }

            QQC2.MenuSeparator {}

            QQC2.MenuItem {
                text: i18nc("'Space' is a matrix space", "Leave Space")
                icon.name: "go-previous"
                onTriggered: RoomManager.leaveRoom(room)
            }

            onClosed: {
                root.closed();
                regularMenu.destroy();
            }
        }
    }

    Component {
        id: mobileMenu

        Kirigami.OverlayDrawer {
            id: drawer
            height: popupContent.implicitHeight
            edge: Qt.BottomEdge
            padding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            topPadding: 0

            parent: applicationWindow().overlay

            ColumnLayout {
                id: popupContent

                width: parent.width
                spacing: 0

                RowLayout {
                    id: headerLayout
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.largeSpacing

                    KirigamiComponents.Avatar {
                        id: avatar
                        source: room.avatarMediaId ? root.room.connection.makeMediaUrl("mxc://" + room.avatarMediaId) : ""
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                        Layout.alignment: Qt.AlignTop
                    }

                    Kirigami.Heading {
                        level: 5
                        Layout.fillWidth: true
                        text: room.displayName
                        wrapMode: Text.WordWrap
                    }
                }

                FormCard.FormButtonDelegate {
                    text: i18nc("'Space' is a matrix space", "View Space")
                    icon.name: "view-list-details"
                    onClicked: RoomManager.resolveResource(root.room.id)
                }

                FormCard.FormButtonDelegate {
                    text: i18nc("@action:inmenu", "Copy Address to Clipboard")
                    icon.name: "edit-copy"
                    onClicked: if (room.canonicalAlias.length === 0) {
                        Clipboard.saveText(room.id);
                    } else {
                        Clipboard.saveText(room.canonicalAlias);
                    }
                }

                FormCard.FormButtonDelegate {
                    text: i18nc("'Space' is a matrix space", "Space Settings")
                    icon.name: 'settings-configure-symbolic'
                    onClicked: {
                        RoomSettingsView.openRoomSettings(root.room, RoomSettingsView.Space);
                        drawer.close();
                    }
                }

                FormCard.FormButtonDelegate {
                    text: i18nc("'Space' is a matrix space", "Leave Space")
                    onClicked: RoomManager.leaveRoom(room)
                }
            }
            onClosed: root.closed()
        }
    }

    asynchronous: true
    sourceComponent: Kirigami.Settings.isMobile ? mobileMenu : regularMenu

    function open() {
        active = true;
    }

    onStatusChanged: if (status == Loader.Ready) {
        if (Kirigami.Settings.isMobile) {
            item.open();
        } else {
            item.popup();
        }
    }
}
