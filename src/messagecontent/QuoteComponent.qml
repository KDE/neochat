// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.TextArea {
    id: root

    /**
     * @brief The index of the delegate in the model.
     */
    required property int index

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
     * @brief Whether the component should be editable.
     */
    required property bool editable

    /**
     * @brief The attributes of the component.
     */
    required property var componentAttributes
    readonly property ChatDocumentHandler chatDocumentHandler: componentAttributes?.chatDocumentHandler ?? null
    onChatDocumentHandlerChanged: if (chatDocumentHandler) {
        chatDocumentHandler.type = ChatBarType.Room;
        chatDocumentHandler.room = root.Message.room;
        chatDocumentHandler.textItem = root;
    }

    /**
     * @brief Whether the component is currently focussed.
     */
    required property bool currentFocus
    onCurrentFocusChanged: if (currentFocus && !focus) {
        forceActiveFocus();
    }

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    Keys.onUpPressed: (event) => {
        event.accepted = false;
        if (root.chatDocumentHandler.atFirstLine) {
            Message.contentModel.focusRow = root.index - 1
        }
    }
    Keys.onDownPressed: (event) => {
        event.accepted = false;
        if (root.chatDocumentHandler.atLastLine) {
            Message.contentModel.focusRow = root.index + 1
        }
    }
    Keys.onLeftPressed: (event) => {
        if (cursorPosition == 1) {
            event.accepted = true;
        } else {
            event.accepted = false;
        }
    }
    Keys.onRightPressed: (event) => {
        if (cursorPosition == (length - 1)) {
            event.accepted = true;
            return;
        }
        event.accepted = false;
    }

    Keys.onDeletePressed: (event) => {
        event.accepted = true;
        chatDocumentHandler.deleteChar();
    }
    Keys.onPressed: (event) => {
        if (event.key == Qt.Key_Backspace) {
            event.accepted = true;
            chatDocumentHandler.backspace();
            return;
        }
        event.accepted = false;
    }

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    text: root.editable ? "" : root.display
    selectByMouse: true
    persistentSelection: true
    readOnly: !root.editable
    textFormat: TextEdit.RichText
    wrapMode: TextEdit.Wrap
    color: Kirigami.Theme.textColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font.italic: true
    font.pointSize: Kirigami.Theme.defaultFont.pointSize * NeoChatConfig.fontScale

    onSelectedTextChanged: root.selectedTextChanged(selectedText)

    onFocusChanged: if (focus && !currentFocus) {
        Message.contentModel.setFocusRow(root.index, true)
    }

    onCursorPositionChanged: if (cursorPosition == 0) {
        cursorPosition = 1;
    } else if (cursorPosition == length) {
        cursorPosition = length - 1;
    }

    TapHandler {
        enabled: !root.hoveredLink
        acceptedDevices: PointerDevice.TouchScreen
        acceptedButtons: Qt.LeftButton
        onLongPressed: {
            const event = root.Message.room.findEvent(root.eventId);
            RoomManager.viewEventMenu(root.QQC2.Overlay.overlay, event, root.Message.room, root.Message.selectedText, root.Message.hoveredLink);
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.alternateBackgroundColor
        radius: Kirigami.Units.cornerRadius
    }
}
