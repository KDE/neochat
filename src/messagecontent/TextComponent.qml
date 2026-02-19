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
    onEditableChanged: manageDefaultMenus()

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
     * @brief Whether the message contains a spoiler
     */
    readonly property var hasSpoiler: root.componentAttributes?.hasSpoiler ?? false

    /**
     * @brief Whether this message is replying to another.
     */
    property bool isReply: false

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    Keys.onPressed: (event) => {
        event.accepted = Message.contentModel.keyHelper.handleKey(event.key, event.modifiers);
    }

    onFocusChanged: if (focus && !currentFocus && editable) {
        Message.contentModel.setFocusRow(index, true)
    }

    ListView.onReused: Qt.binding(() => !hasSpoiler.test(display))

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    text: root.editable ? "" : display

    color: Kirigami.Theme.textColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font {
        pointSize: !root.isReply && QmlUtils.isEmoji(display)
                   ? Kirigami.Theme.defaultFont.pointSize * 4 * NeoChatConfig.fontScale
                   : Kirigami.Theme.defaultFont.pointSize * NeoChatConfig.fontScale
        family: QmlUtils.isEmoji(display) ? 'emoji' : Kirigami.Theme.defaultFont.family
    }
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: !root.editable
    wrapMode: Text.Wrap
    textFormat: Text.RichText
    placeholderText: if (!editable || index !== (Message.contentModel?.hasAttachment ? 1 : 0)) {
        return "";
    } else if (Message.contentModel?.hasAttachment) {
        i18nc("@placeholder", "Set an attachment caption…")
    } else if (Message.room?.usesEncryption) {
        i18nc("@placeholder", "Send an encrypted message…")
    } else {
        i18nc("@placeholder", "Send a message…")
    }

    onLinkActivated: link => {
        if (!root.editable) {
            RoomManager.resolveResource(link, "join");
        }
    }
    onHoveredLinkChanged: if (hoveredLink.length > 0 && hoveredLink !== "1") {
        (QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = hoveredLink;
    } else {
        (QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = "";
    }

    Component.onCompleted: manageDefaultMenus()

    HoverHandler {
        cursorShape: root.hoveredLink || (!(root.componentAttributes?.spoilerRevealed ?? false) && root.hasSpoiler) ? Qt.PointingHandCursor : Qt.IBeamCursor
    }
    TapHandler {
        enabled: !root.hoveredLink && root.hasSpoiler
        onTapped: root.Message.contentModel.toggleSpoiler(root.Message.contentFilterModel.mapToSource(root.Message.contentFilterModel.index(root.index, 0)))
    }
    TapHandler {
        enabled: !root.hoveredLink && !root.editable
        acceptedButtons: Qt.LeftButton
        acceptedDevices: PointerDevice.TouchScreen
        onLongPressed: {
            requestMenu();
        }
    }
    TapHandler {
        enabled: !root.editable
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
        onTapped: {
            requestMenu();
        }
    }

    function requestMenu() {
        const event = root.Message.room.findEvent(root.eventId);
        RoomManager.viewEventMenu(root.QQC2.Overlay.overlay, event, root.Message.room, root.Message.selectedText, root.Message.hoveredLink);
    }

    // TODO - Remove this once the state of TextArea is sorted in QQC2
    // This is horrible I know I hate it but currently seemingly the only way to stop the default
    // menus in TextArea see https://invent.kde.org/frameworks/qqc2-desktop-style/-/issues/15
    function manageDefaultMenus(): void {
        for (let i = 0; i < resources.length; i++) {
            if (resources[i] instanceof TapHandler) {
                (resources[i] as TapHandler).enabled = root.editable;
                return;
            }
        }
    }

    background: null
}
