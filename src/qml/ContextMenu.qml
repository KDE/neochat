// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.settings

/**
 * Context menu when clicking on a room in the room list
 */
Loader {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection

    signal closed

    Component {
        id: regularMenu
        QQC2.Menu {
            QQC2.MenuItem {
                text: i18n("Mark as Read")
                icon.name: "checkmark"
                enabled: room.notificationCount > 0
                onTriggered: room.markAllMessagesAsRead()
            }

            QQC2.MenuSeparator {}

            QQC2.Menu {
                title: i18nc("@action:inmenu", "Notifications")
                icon.name: "notifications"

                QQC2.MenuItem {
                    text: i18n("Follow Global Setting")
                    icon.name: "globe"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.Default
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.Default;
                    }
                }
                QQC2.MenuItem {
                    text: i18nc("As in 'notify for all messages'", "All")
                    icon.name: "notifications"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.All
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.All;
                    }
                }
                QQC2.MenuItem {
                    text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'", "@Mentions and Keywords")
                    icon.name: "im-user"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.MentionKeyword
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.MentionKeyword;
                    }
                }
                QQC2.MenuItem {
                    text: i18nc("As in 'do not notify for any messages'", "Off")
                    icon.name: "notifications-disabled"
                    checkable: true
                    autoExclusive: true
                    checked: room.pushNotificationState === PushNotificationState.Mute
                    enabled: room.pushNotificationState != PushNotificationState.Unknown
                    onTriggered: {
                        room.pushNotificationState = PushNotificationState.Mute;
                    }
                }
            }

            QQC2.MenuItem {
                text: room.isFavourite ? i18n("Remove from Favorites") : i18n("Add to Favorites")
                icon.name: room.isFavourite ? "rating" : "rating-unrated"
                onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
            }

            QQC2.MenuItem {
                text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
                icon.name: room.isLowPriority ? "arrow-up-symbolic" : "arrow-down-symbolic"
                onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
            }

            QQC2.MenuSeparator {}

            QQC2.MenuItem {
                text: room.isDirectChat() ? i18nc("@action:inmenu", "Copy user's Matrix ID to Clipboard") : i18nc("@action:inmenu", "Copy Address to Clipboard")
                icon.name: "edit-copy"
                onTriggered: if (room.isDirectChat()) {
                    Clipboard.saveText(room.directChatRemoteMember.id);
                } else if (room.canonicalAlias.length === 0) {
                    Clipboard.saveText(room.id);
                } else {
                    Clipboard.saveText(room.canonicalAlias);
                }
            }

            QQC2.MenuItem {
                text: i18nc("@action:inmenu", "Room Settings")
                icon.name: 'settings-configure-symbolic'
                onTriggered: {
                    RoomSettingsView.openRoomSettings(root.room, RoomSettingsView.Room);
                }
            }

            QQC2.MenuSeparator {}

            QQC2.MenuItem {
                text: i18n("Leave Room")
                icon.name: "go-previous"
                onTriggered: {
                    Qt.createComponent('org.kde.neochat', 'ConfirmLeaveDialog').createObject(root.QQC2.ApplicationWindow.window, {
                        room: root.room
                    }).open();
                }

            }

            onClosed: {
                root.closed();
            }
        }
    }

    Component {
        id: mobileMenu

        Kirigami.OverlayDrawer {
            id: drawer

            parent: applicationWindow().overlay
            edge: Qt.BottomEdge

            height: popupContent.implicitHeight

            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            topPadding: 0

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
                        source: room.avatarMediaUrl
                        name: room.displayName
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                        Layout.alignment: Qt.AlignTop
                    }
                    Kirigami.Heading {
                        level: 5
                        Layout.fillWidth: true
                        text: room.displayName
                        elide: Text.ElideRight
                    }
                    QQC2.ToolButton {
                        checked: room.isFavourite
                        checkable: true
                        icon.name: 'favorite'
                        Accessible.name: room.isFavourite ? i18n("Remove from Favourites") : i18n("Add to Favourites")
                        onClicked: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
                    }

                    QQC2.ToolButton {
                        text: i18nc("@action:button", "Room Settings")
                        icon.name: 'settings-configure-symbolic'
                        onClicked: {
                            RoomSettingsView.openRoomSettings(root.room, RoomSettingsView.Room);
                            drawer.close();
                        }
                    }
                }

                Delegates.RoundedItemDelegate {
                    text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
                    icon.name: room.isLowPriority ? "arrow-up-symbolic" : "arrow-down-symbolic"
                    onClicked: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
                    Layout.fillWidth: true
                }

                Delegates.RoundedItemDelegate {
                    text: i18n("Mark as Read")
                    icon.name: "checkmark"
                    onClicked: room.markAllMessagesAsRead()
                    Layout.fillWidth: true
                }

                Delegates.RoundedItemDelegate {
                    text: i18n("Leave Room")
                    icon.name: "go-previous"
                    onClicked: {
                        Qt.createComponent('org.kde.neochat', 'ConfirmLeaveDialog').createObject(root.QQC2.ApplicationWindow.window, {
                            room: root.room
                        }).open();
                        drawer.close();
                    }
                    Layout.fillWidth: true
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
