// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
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

    title: i18nc("@action:title", "Pinned Messages")

    ListView {
        id: listView
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0

        model: PinnedMessagesModel {
            id: searchModel
            room: root.room
        }

        delegate: EventDelegate {
            room: root.room
        }

        section.property: "section"

        Kirigami.PlaceholderMessage {
            id: noSearchMessage
            anchors.centerIn: parent
            text: i18n("No Pinned Messages")
            visible: listView.count === 0
        }

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: listView.count === 0 && root.model.loading
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
