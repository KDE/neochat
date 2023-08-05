// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

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

        visible: root.currentRoom.canSendEvent("m.room.message")

        Layout.fillWidth: true
        Layout.minimumHeight: Math.max(Kirigami.Units.gridUnit * 2, implicitHeight + Kirigami.Units.largeSpacing)
        // lineSpacing is height+leading, so subtract leading once since leading only exists between lines.
        Layout.maximumHeight: chatBarFontMetrics.lineSpacing * 8 - chatBarFontMetrics.leading + textField.topPadding + textField.bottomPadding

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
