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
KirigamiComponents.ConvergentContextMenu {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection

    headerContentItem: RowLayout {
        id: headerLayout
        Layout.fillWidth: true
        spacing: Kirigami.Units.largeSpacing
        KirigamiComponents.Avatar {
            id: avatar
            source: room.avatarMediaUrl
            name: room.displayName
            Layout.preferredWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            Layout.preferredHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            Layout.alignment: Qt.AlignTop
        }
        Kirigami.Heading {
            level: 5
            Layout.fillWidth: true
            text: room.displayName
            elide: Text.ElideRight
        }
    }

    QQC2.Action {
        text: i18n("Mark as Read")
        icon.name: "checkmark"
        enabled: room.notificationCount > 0
        onTriggered: room.markAllMessagesAsRead()
    }

    Kirigami.Action {
        separator: true
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Notifications")
        icon.name: "notifications"

        Kirigami.Action {
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

        Kirigami.Action {
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

        Kirigami.Action {
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

        Kirigami.Action {
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

    QQC2.Action {
        text: room.isFavourite ? i18n("Remove from Favorites") : i18n("Add to Favorites")
        icon.name: room.isFavourite ? "rating" : "rating-unrated"
        onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
    }

    QQC2.Action {
        text: room.isLowPriority ? i18n("Reprioritize") : i18n("Deprioritize")
        icon.name: room.isLowPriority ? "arrow-up-symbolic" : "arrow-down-symbolic"
        onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
    }

    Kirigami.Action {
        separator: true
    }

    QQC2.Action {
        text: room.isDirectChat() ? i18nc("@action:inmenu", "Copy user's Matrix ID") : i18nc("@action:inmenu", "Copy Room Address")
        icon.name: "edit-copy"
        onTriggered: {
            // The canonical alias (if it exists) otherwise the first available alias
            const firstAlias = room.aliases[0];

            if (room.isDirectChat()) {
                Clipboard.saveText(room.directChatRemoteMember.id);
            } else if (!firstAlias) {
                Clipboard.saveText(room.id);
            } else {
                Clipboard.saveText(firstAlias);
            }
        }
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Room Settings")
        icon.name: 'settings-configure-symbolic'
        onTriggered: {
            RoomSettingsView.openRoomSettings(root.room, RoomSettingsView.Room);
        }
    }

    Kirigami.Action {
        separator: true
    }

    QQC2.Action {
        text: i18n("Leave Room")
        icon.name: "go-previous"
        onTriggered: {
            Qt.createComponent('org.kde.neochat', 'ConfirmLeaveDialog').createObject(root.QQC2.ApplicationWindow.window, {
                room: root.room
            }).open();
        }
    }
}
