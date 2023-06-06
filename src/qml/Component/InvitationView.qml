// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

Kirigami.PlaceholderMessage {
    id: root

    required property NeoChatRoom currentRoom

    text: i18n("Accept this invitation?")
    RowLayout {
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
