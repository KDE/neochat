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
    readonly property ChatTextItemHelper chatTextItemHelper: componentAttributes?.chatTextItemHelper ?? null
    onChatTextItemHelperChanged: if (chatTextItemHelper) {
        chatTextItemHelper.textItem = root;
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

    Keys.onPressed: (event) => {
        event.accepted = Message.contentModel.keyHelper.handleKey(event.key, event.modifiers);
    }

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    text: root.editable ? "" : root.display
    selectByMouse: true
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
