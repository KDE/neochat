// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.settings
import org.kde.neochat.devtools

KirigamiComponents.ConvergentContextMenu {
    id: root

    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow window
    required property var author

    QQC2.Action {
        text: i18nc("@action:button", "Open Profile")
        icon.name: "im-user-symbolic"
        onTriggered: RoomManager.resolveResource(root.author.uri)
    }

    QQC2.Action {
        text: i18nc("@action:button", "Mention")
        icon.name: "username-copy-symbolic"
        onTriggered: {
            RoomManager.currentRoom.mainCache.mentionAdded(root.author.id);
        }
    }
}
