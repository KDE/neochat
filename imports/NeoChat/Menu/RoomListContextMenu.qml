// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Page 1.0

/**
 * Context menu when clicking on a room in the room list
 */
Loader {
    id: root
    property var room
    signal closed()

    Component {
        id: regularMenu
        Menu {
            MenuItem {
                id: newWindow
                text: i18n("Open in new window")
                onTriggered: RoomManager.openWindow(room);
                visible: !Kirigami.Settings.isMobile
            }

            MenuSeparator {
                visible: newWindow.visible
            }

            MenuItem {
                text: room.isFavourite ? i18n("Remove from Favourites") : i18n("Add to Favourites")
                onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
            }

            MenuItem {
                text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
                onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
            }

            MenuItem {
                text: i18n("Mark as Read")
                onTriggered: room.markAllMessagesAsRead()
            }

            MenuItem {
                text: i18nc("@action:inmenu", "Copy address to clipboard")
                onTriggered: if (room.canonicalAlias.length === 0) {
                    Clipboard.saveText(room.id)
                } else {
                    Clipboard.saveText(room.canonicalAlias)
                }
            }

            MenuItem {
                text: i18n("Room settings")
                onTriggered: ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/imports/NeoChat/RoomSettings/Categories.qml', {room: room})
            }

            MenuSeparator {}

            MenuItem {
                text: i18n("Leave Room")
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
                    Kirigami.Avatar {
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
                    ToolButton {
                        checked: room.isFavourite
                        checkable: true
                        icon.name: 'favorite'
                        Accessible.name: room.isFavourite ? i18n("Remove from Favourites") : i18n("Add to Favourites")
                        onClicked: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
                    }

                    ToolButton {
                        icon.name: 'settings-configure'
                        onClicked: ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/imports/NeoChat/RoomSettings/Categories.qml', {room: room})
                    }
                }

                Kirigami.BasicListItem {
                    text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
                    onClicked: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
                    implicitHeight: visible ? Kirigami.Units.gridUnit * 3 : 0
                }

                Kirigami.BasicListItem {
                    text: i18n("Mark as Read")
                    onClicked: room.markAllMessagesAsRead()
                    implicitHeight: visible ? Kirigami.Units.gridUnit * 3 : 0
                }
                Kirigami.BasicListItem {
                    text: i18n("Leave Room")
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
