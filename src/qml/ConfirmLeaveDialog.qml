// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property NeoChatRoom room

    width: Kirigami.Units.gridUnit * 24

    title: i18nc("@title:dialog", "Confirm Leaving Room")

    contentItem: FormCard.FormTextDelegate {
        text: root.room ? i18nc("Do you really want to leave <room name>?", "Do you really want to leave %1?", root.room.displayName) : ""
    }

    customFooterActions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Leave Room")
            icon.name: "arrow-left"
            onTriggered: RoomManager.leaveRoom(root.room)
        }
    ]
}
