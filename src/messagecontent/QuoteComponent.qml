// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.Control {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property NeochatRoomMember author

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: Message.maxContentWidth

    topPadding: 0
    bottomPadding: 0

    contentItem: TextEdit {
        id: quoteText
        Layout.fillWidth: true
        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing

        text: root.display
        readOnly: true
        textFormat: TextEdit.RichText
        wrapMode: TextEdit.Wrap
        color: Kirigami.Theme.textColor

        font.italic: true

        onSelectedTextChanged: root.selectedTextChanged(selectedText)

        TapHandler {
            enabled: !quoteText.hoveredLink
            acceptedDevices: PointerDevice.TouchScreen
            acceptedButtons: Qt.LeftButton
            onLongPressed: RoomManager.viewEventMenu(root.eventId, root.Message.room, root.author, root.Message.selectedText, root.Message.hoveredLink);
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius
    }
}
