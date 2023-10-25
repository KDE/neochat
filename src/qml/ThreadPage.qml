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
     * @brief The model to display in the timeline.
     *
     * The timeline expects a model which supports the same set of roles as
     * MessageEventModel in order for all delegate types to work. The model must also
     * have a loading Q_PROPERTY.
     */
    property alias model: timeline.model

    /**
     * @brief The ActionsHandler object to use.
     */
    property ActionsHandler actionsHandler: ActionsHandler {
        room: root.room
    }

    SimpleTimelineView {
        id: timeline
        anchors.fill: parent
        connection: root.room.connection
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
            Component.onCompleted: chatBarCache.threadId = root.model.threadRootId

            // onMessageSent: {
            //     if (!timelineViewLoader.item.atYEnd) {
            //         timelineViewLoader.item.goToLastMessage();
            //     }
            // }
        }
    }
}
