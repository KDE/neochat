// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.PlaceholderMessage {
    id: root

    required property NeoChatRoom currentRoom

    text: i18n("Accept this invitation?")
    RowLayout {
        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            text: i18nc("@action:button The thing being rejected is an invitation to chat", "Reject and ignore user")

            onClicked: {
                RoomManager.leaveRoom(root.currentRoom);
                root.currentRoom.connection.addToIgnoredUsers(root.currentRoom.invitingUser());
            }
        }
        QQC2.Button {
            Layout.alignment : Qt.AlignHCenter
            text: i18n("Reject")

            onClicked: RoomManager.leaveRoom(root.currentRoom);
        }

        QQC2.Button {
            Layout.alignment : Qt.AlignHCenter
            text: i18n("Accept")

            onClicked: {
                root.currentRoom.acceptInvitation();
            }
        }
    }
}
