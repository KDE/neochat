// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0

/**
 * Context menu when clicking on a room in the room list
 */
Loader {
    id: root
    property NeoChatRoom room
    signal closed()

    Component {
        id: regularMenu
        QQC2.Menu {
            QQC2.MenuItem {
                text: i18nc("'Space' is a matrix space", "View Space")
                icon.name: "expand"
                onTriggered: RoomManager.enterRoom(room);
            }

            QQC2.MenuItem {
                text: i18nc("@action:inmenu", "Copy Address to Clipboard")
                icon.name: "edit-copy"
                onTriggered: if (room.canonicalAlias.length === 0) {
                    Clipboard.saveText(room.id)
                } else {
                    Clipboard.saveText(room.canonicalAlias)
                }
            }

            QQC2.MenuItem {
                text: i18nc("'Space' is a matrix space", "Space Settings")
                icon.name: 'settings-configure'
                onTriggered: QQC2.ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/Categories.qml', {room: room}, { title: i18n("Space Settings") })
            }

            QQC2.MenuSeparator {}

            QQC2.MenuItem {
                text: i18nc("'Space' is a matrix space", "Leave Space")
                icon.name: "go-previous"
                onTriggered: RoomManager.leaveRoom(room)
            }

            onClosed: {
                root.closed()
                destroy()
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
                        source: room.avatarMediaId ? ("image://mxc/" + room.avatarMediaId) : ""
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

                    QQC2.ToolButton {
                        icon.name: 'settings-configure'
                        onClicked: QQC2.ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/Categories.qml', {room: room}, { title: i18n("Space Settings") })
                    }
                }
                Kirigami.BasicListItem {
                    text: i18nc("'Space' is a matrix space", "Leave Space")
                    onClicked: RoomManager.leaveRoom(room)
                    implicitHeight: visible ? Kirigami.Units.gridUnit * 3 : 0
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
