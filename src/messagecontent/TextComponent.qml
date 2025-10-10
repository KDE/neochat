// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show rich text from a message.
 */
TextEdit {
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
     * @brief Whether the message contains a spoiler
     */
    readonly property var hasSpoiler: root.componentAttributes?.hasSpoiler ?? false

    /**
     * @brief Whether this message is replying to another.
     */
    property bool isReply: false

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    Keys.onUpPressed: (event) => {
        event.accepted = false;
        if (chatDocumentHandler.atFirstLine) {
            Message.contentModel.focusRow = root.index - 1
        }
    }
    Keys.onDownPressed: (event) => {
        event.accepted = false;
        if (chatDocumentHandler.atLastLine) {
            Message.contentModel.focusRow = root.index + 1
        }
    }

    Keys.onDeletePressed: (event) => {
        event.accepted = true;
        chatDocumentHandler.deleteChar();
    }

    Keys.onPressed: (event) => {
        if (event.key == Qt.Key_Backspace && cursorPosition == 0) {
            event.accepted = true;
            chatDocumentHandler.backspace();
            return;
        }
        event.accepted = false;
    }

    onFocusChanged: if (focus && !root.currentFocus) {
        Message.contentModel.setFocusRow(root.index, true)
    }

    ListView.onReused: Qt.binding(() => !hasSpoiler.test(display))

    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    persistentSelection: true

    text: root.editable ? "" : display

    color: Kirigami.Theme.textColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    // font {
    //     pointSize: !root.isReply && QmlUtils.isEmoji(display) ? Kirigami.Theme.defaultFont.pointSize * 4 : Kirigami.Theme.defaultFont.pointSize
    //     family: QmlUtils.isEmoji(display) ? 'emoji' : Kirigami.Theme.defaultFont.family
    // }
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: !root.editable
    wrapMode: Text.Wrap
    textFormat: Text.RichText

    onLinkActivated: link => {
        RoomManager.resolveResource(link, "join");
    }
    onHoveredLinkChanged: if (hoveredLink.length > 0 && hoveredLink !== "1") {
        (QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = hoveredLink;
    } else {
        (QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = "";
    }

    HoverHandler {
        cursorShape: root.hoveredLink || (!(root.componentAttributes?.spoilerRevealed ?? false) && root.hasSpoiler) ? Qt.PointingHandCursor : Qt.IBeamCursor
    }
    TapHandler {
        enabled: !root.hoveredLink && root.hasSpoiler
        onTapped: root.Message.contentModel.toggleSpoiler(root.Message.contentFilterModel.mapToSource(root.Message.contentFilterModel.index(root.index, 0)))
    }
    TapHandler {
        enabled: !root.hoveredLink
        acceptedButtons: Qt.LeftButton
        acceptedDevices: PointerDevice.TouchScreen
        onLongPressed: RoomManager.viewEventMenu(root.eventId, root.Message.room, root.Message.selectedText, root.Message.hoveredLink);
    }
    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
        gesturePolicy: TapHandler.WithinBounds
        onTapped: RoomManager.viewEventMenu(root.eventId, root.Message.room, root.Message.selectedText, root.Message.hoveredLink);
    }
}
