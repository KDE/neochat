// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.ki18n

TimelineDelegate {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom room

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: Kirigami.InlineMessage {
        visible: true
        text: KI18n.i18n("This room continues another conversation.")
        type: Kirigami.MessageType.Information
        actions: Kirigami.Action {
            text: KI18n.i18n("See older messages…")
            onTriggered: RoomManager.resolveResource(root.room.predecessorId)
        }
    }
}
