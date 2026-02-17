// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import Quotient

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
            source: root.room.avatarMediaUrl
            name: root.room.displayName
            Layout.preferredWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            Layout.preferredHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            Layout.alignment: Qt.AlignTop
        }
        Kirigami.Heading {
            level: 5
            Layout.fillWidth: true
            text: root.room.displayName
            elide: Text.ElideRight
        }
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Mark as Read")
        icon.name: "checkmark"
        enabled: root.room.notificationCount > 0 || root.room.highlightCount > 0
        onTriggered: root.room.markAllMessagesAsRead(NeoChatConfig.publicReadReceipts)
    }

    Kirigami.Action {
        separator: true
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Notifications")
        icon.name: "notifications"

        Kirigami.Action {
            text: i18nc("@action:inmenu Notification 'Default Settings'", "Default Settings")
            icon.name: "globe"
            checkable: true
            autoExclusive: true
            checked: root.room.pushNotificationState === PushNotificationState.Default
            enabled: root.room.pushNotificationState != PushNotificationState.Unknown
            onTriggered: {
                root.room.pushNotificationState = PushNotificationState.Default;
            }
        }

        Kirigami.Action {
            text: i18nc("As in 'notify for all messages'", "All Messages")
            icon.name: "notifications"
            checkable: true
            autoExclusive: true
            checked: root.room.pushNotificationState === PushNotificationState.All
            enabled: root.room.pushNotificationState != PushNotificationState.Unknown
            onTriggered: {
                root.room.pushNotificationState = PushNotificationState.All;
            }
        }

        Kirigami.Action {
            text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'", "@Mentions and Keywords")
            icon.name: "im-user"
            checkable: true
            autoExclusive: true
            checked: root.room.pushNotificationState === PushNotificationState.MentionKeyword
            enabled: root.room.pushNotificationState != PushNotificationState.Unknown
            onTriggered: {
                root.room.pushNotificationState = PushNotificationState.MentionKeyword;
            }
        }

        Kirigami.Action {
            text: i18nc("As in 'do not notify for any messages'", "None")
            icon.name: "notifications-disabled"
            checkable: true
            autoExclusive: true
            checked: root.room.pushNotificationState === PushNotificationState.Mute
            enabled: root.room.pushNotificationState != PushNotificationState.Unknown
            onTriggered: {
                root.room.pushNotificationState = PushNotificationState.Mute;
            }
        }
    }

    QQC2.Action {
        text: root.room.isFavourite ? i18nc("@action:inmenu", "Remove from Favorites") : i18nc("@action:inmenu", "Add to Favorites")
        icon.name: root.room.isFavourite ? "rating" : "rating-unrated"
        onTriggered: root.room.isFavourite ? root.room.removeTag("m.favourite") : root.room.addTag("m.favourite", 1.0)
    }

    QQC2.Action {
        text: root.room.isLowPriority ? i18nc("@action:inmenu", "Reprioritize") : i18nc("@action:inmenu", "Deprioritize")
        icon.name: root.room.isLowPriority ? "arrow-up-symbolic" : "arrow-down-symbolic"
        onTriggered: root.room.isLowPriority ? root.room.removeTag("m.lowpriority") : root.room.addTag("m.lowpriority", 1.0)
    }

    Kirigami.Action {
        separator: true
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Copy Room Link")
        icon.name: "edit-copy"
        visible: !root.room.isDirectChat() && root.room.joinRule !== JoinRule.Invite
        onTriggered: {
            // The canonical alias (if it exists) otherwise the first available alias
            const firstAlias = root.room.aliases[0];
            if (firstAlias) {
                Clipboard.saveText("https://matrix.to/#/" + firstAlias);
            } else {
                Clipboard.saveText("https://matrix.to/#/" + root.room.id);
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

    Kirigami.Action {
        text: i18nc("@action:button 'Report' as in 'Report this room to the administrators'", "Report…")
        icon.name: "dialog-warning-symbolic"
        visible: root.connection.supportsMatrixSpecVersion("v1.13")
        onTriggered: {
            let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                title: i18nc("@title:dialog", "Report Room"),
                placeholder: i18nc("@info:placeholder", "Optionally give a reason for reporting this room"),
                icon: "dialog-warning-symbolic",
                actionText: i18nc("@action:button 'Report' as in 'Report this room to the administrators'", "Report"),
                reporting: true,
                connection: root.connection,
            }, {
                title: i18nc("@title", "Report Room"),
                width: Kirigami.Units.gridUnit * 25
            }) as ReasonDialog;
            dialog.accepted.connect(reason => {
                currentRoom.report(reason);
            });
        }
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Leave Room…")
        icon.name: "go-previous"
        onTriggered: {
            (Qt.createComponent('org.kde.neochat', 'ConfirmLeaveDialog').createObject(root.QQC2.ApplicationWindow.window, {
                room: root.room
            }) as ConfirmLeaveDialog).open();
        }
    }
}
