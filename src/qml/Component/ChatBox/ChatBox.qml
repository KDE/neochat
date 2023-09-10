// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.neochat

/**
 * @brief A component for typing and sending chat messages.
 *
 * This is designed to go to the bottom of the timeline and provides all the functionality
 * required for the user to send messages to the room.
 *
 * This includes support for the following message types:
 *  - text
 *  - media (video, image, file)
 *  - emojis/stickers
 *  - location
 *
 * In addition when replying this component supports showing the message that is being
 * replied to.
 *
 * @note The main role of this component is to layout the elements. The main functionality
 *       is handled by ChatBar
 *
 * @sa ChatBar
 */
ColumnLayout {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    required property NeoChatConnection connection

    /**
     * @brief A message has been sent from the chat bar.
     */
    signal messageSent()

    /**
     * @brief Insert the given text into the ChatBar.
     *
     * The text is inserted at the current cursor location.
     */
    function insertText(text) {
        chatBar.insertText(text)
    }

    spacing: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    ChatBar {
        id: chatBar

        connection: root.connection

        visible: root.currentRoom.canSendEvent("m.room.message")

        Layout.fillWidth: true
        Layout.minimumHeight: Math.max(Kirigami.Units.gridUnit * 2, Math.round(implicitHeight) + Kirigami.Units.largeSpacing)
        // lineSpacing is height+leading, so subtract leading once since leading only exists between lines.
        Layout.maximumHeight: chatBarFontMetrics.lineSpacing * 8 - chatBarFontMetrics.leading + textField.topPadding + textField.bottomPadding
        Layout.preferredHeight: Math.round(implicitHeight)

        currentRoom: root.currentRoom

        FontMetrics {
            id: chatBarFontMetrics
            font: chatBar.textField.font
        }

        onMessageSent: {
            root.messageSent();
        }
    }

    onActiveFocusChanged: chatBar.forceActiveFocus()
}
