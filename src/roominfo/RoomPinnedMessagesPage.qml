// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat
import org.kde.neochat.timeline

/**
 * @brief Component for showing the pinned messages in a room.
 */
Kirigami.ScrollablePage {
    id: root

    /**
     * @brief The room to show the pinned messages for.
     */
    required property NeoChatRoom room

    title: i18nc("@title", "Pinned Messages")

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    ListView {
        id: listView
        spacing: 0

        model: PinnedMessageModel {
            id: pinModel
            room: root.room
        }

        delegate: EventDelegate {
            room: root.room
        }

        section.property: "section"

        Kirigami.PlaceholderMessage {
            icon.name: "pin-symbolic"
            anchors.centerIn: parent
            text: i18nc("@info:placeholder", "No Pinned Messages")
            visible: listView.count === 0
        }

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: listView.count === 0 && pinModel.loading
        }

        Keys.onUpPressed: {
            if (listView.currentIndex > 0) {
                listView.decrementCurrentIndex();
            } else {
                listView.currentIndex = -1; // This is so the list view doesn't appear to have two selected items
                listView.headerItem.forceActiveFocus(Qt.TabFocusReason);
            }
        }
    }
}
