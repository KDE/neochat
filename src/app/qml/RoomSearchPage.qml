// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.neochat
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

    title: i18nc("@action:title", "Search Messages")

    model: SearchModel {
        id: searchModel
        room: root.room
    }

    modelDelegate: EventDelegate {
        room: root.room
    }

    searchFieldPlaceholder: i18n("Find messages…")
    noSearchPlaceholderMessage: i18n("Enter text to start searching")
    noResultPlaceholderMessage: i18n("No messages found")

    listVerticalLayoutDirection: ListView.BottomToTop
}
