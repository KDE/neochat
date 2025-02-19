// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatRoom currentRoom
    readonly property var invitingMember: currentRoom.member(currentRoom.invitingUserId())

    spacing: Kirigami.Units.smallSpacing

    KirigamiComponents.Avatar {
        id: avatar
        Layout.preferredWidth: Kirigami.Units.iconSizes.huge
        Layout.preferredHeight: Kirigami.Units.iconSizes.huge
        Layout.alignment: Qt.AlignHCenter

        name: root.invitingMember.displayName
        source: root.invitingMember.avatarUrl
        color: root.invitingMember.color
    }

    Loader {
        active: !root.currentRoom.isDirectChat()

        Layout.alignment: Qt.AlignHCenter

        sourceComponent: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18nc("@info:label", "%1 has invited you to join", root.invitingMember.displayName)

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

        sourceComponent: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: root.currentRoom.displayName

                Layout.alignment: Qt.AlignHCenter
            }

            QQC2.Label {
                text: i18nc("@info:label", "This user is inviting you to chat.")

                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

    QQC2.Label {
        color: Kirigami.Theme.disabledTextColor
        text: i18n("You can reject invitations from unknown users under Security settings.")
        visible: root.currentRoom.connection.canCheckMutualRooms
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: Kirigami.Units.mediumSpacing

        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            text: i18nc("@action:button The thing being rejected is an invitation to chat", "Reject and Ignore User")
            icon.name: "list-remove-symbolic"

            onClicked: {
                RoomManager.leaveRoom(root.currentRoom);
                root.currentRoom.connection.addToIgnoredUsers(root.currentRoom.invitingUserId());
            }
        }
        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            icon.name: "cards-block-symbolic"
            text: i18nc("@action:button", "Reject")

            onClicked: RoomManager.leaveRoom(root.currentRoom)
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            icon.name: "dialog-ok-symbolic"
            text: i18nc("@action:button", "Accept")

            onClicked: root.currentRoom.acceptInvitation()
        }
    }
}
