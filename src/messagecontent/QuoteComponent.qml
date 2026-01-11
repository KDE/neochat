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
        selectedTextColor: Kirigami.Theme.highlightedTextColor
        selectionColor: Kirigami.Theme.highlightColor

        font.italic: true
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * NeoChatConfig.fontScale

        onSelectedTextChanged: root.selectedTextChanged(selectedText)

        TapHandler {
            enabled: !quoteText.hoveredLink
            acceptedDevices: PointerDevice.TouchScreen
            acceptedButtons: Qt.LeftButton
            onLongPressed: {
                const event = root.Message.room.findEvent(root.eventId);
                RoomManager.viewEventMenu(root.QQC2.Overlay.overlay, event, root.Message.room, root.Message.selectedText, root.Message.hoveredLink);
            }
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius
    }
}
