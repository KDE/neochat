// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami

import org.kde.neochat 1.0

/**
 * Context menu when clicking on a room in the room list
 */
Loader {
    id: root
    property var room
    signal closed()

    Component {
        id: regularMenu
        QQC2.Menu {
            QQC2.MenuItem {
                id: newWindow
                text: i18n("Open in New Window")
                icon.name: "window-new"
                onTriggered: RoomManager.openWindow(room);
                visible: !Kirigami.Settings.isMobile
            }

            QQC2.MenuSeparator {
                visible: newWindow.visible
            }

            QQC2.MenuItem {
                text: room.isFavourite ? i18n("Remove from Favourites") : i18n("Add to Favourites")
                icon.name: room.isFavourite ? "bookmark-remove" : "bookmark-new"
                onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
            }

            QQC2.MenuItem {
                text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
                icon.name: room.isLowPriority ? "arrow-up" : "arrow-down"
                onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
            }

            QQC2.MenuItem {
                text: i18n("Mark as Read")
                icon.name: "checkmark"
                onTriggered: room.markAllMessagesAsRead()
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

            QQC2.Menu {
                title: i18n("Notification State")

                QQC2.MenuItem {
                    text: i18n("Follow Global Setting")
                    icon.name: "globe"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.Default
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.Default
                    }
                }
                QQC2.MenuItem {
                    text: i18nc("As in 'notify for all messages'","All")
                    icon.name: "notifications"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.All
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.All
                    }
                }
                QQC2.MenuItem {
                    text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'","@Mentions and Keywords")
                    icon.name: "im-user"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.MentionKeyword
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.MentionKeyword
                    }
                }
                QQC2.MenuItem {
                    text: i18nc("As in 'do not notify for any messages'","Off")
                    icon.name: "notifications-disabled"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.Mute
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.Mute
                    }
                }
            }

            QQC2.MenuItem {
                text: i18n("Room Settings")
                icon.name: "configure"
                onTriggered: ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/Categories.qml', {room: room})
            }

            QQC2.MenuSeparator {}

            QQC2.MenuItem {
                text: i18n("Leave Room")
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
                    QQC2.ToolButton {
                        checked: room.isFavourite
                        checkable: true
                        icon.name: 'favorite'
                        Accessible.name: room.isFavourite ? i18n("Remove from Favourites") : i18n("Add to Favourites")
                        onClicked: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
                    }

                    QQC2.ToolButton {
                        icon.name: 'settings-configure'
                        onClicked: ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/Categories.qml', {room: room})
                    }
                }

                Kirigami.BasicListItem {
                    text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
                    icon: room.isLowPriority ? "arrow-up" : "arrow-down"
                    onClicked: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
                    implicitHeight: visible ? Kirigami.Units.gridUnit * 3 : 0
                }

                Kirigami.BasicListItem {
                    text: i18n("Mark as Read")
                    icon: "checkmark"
                    onClicked: room.markAllMessagesAsRead()
                    implicitHeight: visible ? Kirigami.Units.gridUnit * 3 : 0
                }
                Kirigami.BasicListItem {
                    text: i18n("Leave Room")
                    icon: "go-previous"
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
