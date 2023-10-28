// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.neochat

QQC2.Page {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom room

    /**
     * @brief The ActionsHandler object to use.
     */
    property ActionsHandler actionsHandler: ActionsHandler {
        room: root.room
    }

    SimpleTimelineView {
        id: timeline
        anchors.fill: parent
        room: root.room
        model: RoomManager.threadModel
        actionsHandler: root.actionsHandler
    }

    footer: Loader {
        id: chatBoxLoader
        active: !timeline.model.loading
        sourceComponent: ChatBox {
            width: parent.width
            currentRoom: root.room
            connection: root.room.connection
            actionsHandler: root.actionsHandler
            chatBarCache: root.room.threadCache
            Component.onCompleted: chatBarCache.threadId = RoomManager.threadModel.threadRootId

            // onMessageSent: {
            //     if (!timelineViewLoader.item.atYEnd) {
            //         timelineViewLoader.item.goToLastMessage();
            //     }
            // }
        }
    }
}
