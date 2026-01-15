// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat
import org.kde.neochat.timeline

/**
 * @brief Component for finding messages in a room.
 *
 * This component is based on a SearchPage and allows the user to enter a search
 * term into the input field and then search the room for messages with text that
 * matches the input.
 *
 * @sa SearchPage
 */
SearchPage {
    id: root

    /**
     * @brief The room the search is being performed in.
     */
    required property NeoChatRoom room

    /**
     * @brief If set, limits the search to events from a specific user id.
     */
    property string senderId

    title: i18nc("@action:title", "Search Messages")

    model: SearchModel {
        id: searchModel
        room: root.room
        senderId: root.senderId
    }

    modelDelegate: EventDelegate {
        room: root.room
    }

    searchFieldPlaceholder: i18n("Find messages…")
    noSearchPlaceholderMessage: i18n("Enter text to start searching")
    noResultPlaceholderMessage: i18n("No messages found")

    listVerticalLayoutDirection: ListView.BottomToTop

    Connections {
        target: root.room.mainCache

        function onRelationIdChanged(oldEventId: string, newEventId: string): void {
            // If we start replying/editing an event, we need to close the search dialog so the user can type.
            if (newEventId.length > 0) {
                root.Kirigami.PageStack.closeDialog();
            }
        }
    }
}
