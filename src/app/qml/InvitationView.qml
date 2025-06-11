// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatRoom currentRoom
    readonly property var invitingMember: currentRoom.qmlSafeMember(currentRoom.invitingUserId)
    readonly property string inviteTimestamp: root.currentRoom.inviteTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)

    spacing: Kirigami.Units.smallSpacing

    Item {
        Layout.fillHeight: true
    }

    KirigamiComponents.Avatar {
        id: avatar
        Layout.preferredWidth: Kirigami.Units.iconSizes.huge
        Layout.preferredHeight: Kirigami.Units.iconSizes.huge
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true

        name: root.invitingMember.displayName
        source: NeoChatConfig.hideImages ? undefined : root.invitingMember.avatarUrl
        color: root.invitingMember.color
    }

    Loader {
        active: !root.currentRoom.isDirectChat()

        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true

        sourceComponent: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18nc("@info:label 'Username' has invited you to this room at 'timestamp'.", "%1 has invited you to this room at %2.", root.invitingMember.displayName, root.inviteTimestamp)

                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.Heading {
                text: root.currentRoom.displayName

                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

    Loader {
        active: root.currentRoom.isDirectChat()

        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true

        sourceComponent: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: root.currentRoom.displayName

                Layout.alignment: Qt.AlignHCenter
            }

            QQC2.Label {
                text: i18nc("@info:label This user invited you to chat at 'timestamp'", "This user invited you to chat at %1.", root.inviteTimestamp)

                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: acceptInviteDelegate

            icon.name: "dialog-ok-symbolic"
            text: i18nc("@action:button Accept this invite", "Accept Invite")
            focus: true

            onClicked: root.currentRoom.acceptInvitation()
        }

        FormCard.FormDelegateSeparator {
            above: acceptInviteDelegate
            below: rejectInviteDelegate
        }

        FormCard.FormButtonDelegate {
            id: rejectInviteDelegate

            icon.name: "dialog-cancel-symbolic"
            text: i18nc("@action:button Reject this invite", "Reject Invite")

            onClicked: root.currentRoom.forget()
        }
    }

    FormCard.FormCard {
        id: blockUserCard

        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            icon.name: "list-remove-symbolic"
            text: i18nc("@action:button Block the user", "Block %1", root.invitingMember.displayName)

            onClicked: {
                root.currentRoom.forget()
                root.currentRoom.connection.addToIgnoredUsers(root.currentRoom.invitingUserId);
            }
        }
    }

    RowLayout {
        visible: root.currentRoom.connection.canCheckMutualRooms
        spacing: 0

        Layout.topMargin: Kirigami.Units.largeSpacing * 2
        Layout.fillWidth: true

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Item {
                Layout.fillWidth: true
            }

            Kirigami.Icon {
                source: "help-hint-symbolic"
                color: Kirigami.Theme.disabledTextColor

                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            QQC2.Label {
                color: Kirigami.Theme.disabledTextColor
                text: i18nc("@info:label", "You can reject invitations from unknown users under Security settings.")
                wrapMode: Text.WordWrap

                // + 5 to prevent it from wrapping unnecessarily
                Layout.maximumWidth: implicitWidth + 5
                Layout.fillWidth: true
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
