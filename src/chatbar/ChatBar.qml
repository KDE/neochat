// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.libneochat as LibNeoChat

/**
 * @brief A component for typing and sending chat messages.
 *
 * This is designed to go to the bottom of the timeline and provides all the functionality
 * required for the user to send messages to the room.
 *
 * In addition when replying this component supports showing the message that is being
 * replied to.
 *
 * @sa ChatBar
 */
QQC2.Control {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    required property NeoChatConnection connection

    onActiveFocusChanged: textField.forceActiveFocus()

    onCurrentRoomChanged: {
        _private.chatBarCache = currentRoom.mainCache
        if (ShareHandler.text.length > 0 && ShareHandler.room === root.currentRoom.id) {
            textField.text = ShareHandler.text;
            ShareHandler.text = "";
            ShareHandler.room = "";
        }
    }

    Connections {
        target: ShareHandler
        function onRoomChanged(): void {
            if (ShareHandler.text.length > 0 && ShareHandler.room === root.currentRoom.id) {
                textField.text = ShareHandler.text;
                ShareHandler.text = "";
                ShareHandler.room = "";
            }
        }
    }

    Connections {
        target: root.currentRoom.mainCache

        function onMentionAdded(mention: string): void {
            // add mention text
            textField.append(mention + " ");
            // move cursor to the end
            textField.cursorPosition = textField.text.length;
            // move the focus back to the chat bar
            textField.forceActiveFocus(Qt.OtherFocusReason);
        }
    }

    spacing: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Message.room: root.currentRoom
    Message.contentModel: contentModel

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Separator {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
        }
    }

    height: Math.max(Math.min(chatScrollView.contentHeight + bottomPadding + topPadding, Kirigami.Units.gridUnit * 10), Kirigami.Units.gridUnit * 5)
    leftPadding: rightPadding
    rightPadding: (root.width - chatBarSizeHelper.availableWidth) / 2
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    contentItem: QQC2.ScrollView {
        id: chatScrollView
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                id: chatContentView
                model: ChatBarMessageContentModel {
                    id: contentModel
                    type: ChatBarType.Room
                    room: root.currentRoom
                }

                delegate: MessageComponentChooser {}
            }
            RichEditBar {
                id: richEditBar
                maxAvailableWidth: chatBarSizeHelper.availableWidth - Kirigami.Units.largeSpacing * 2

                room: root.currentRoom
                contentModel: chatContentView.model

                onClicked: contentModel.refocusCurrentComponent()
                onRequestPostMessage: _private.postMessage()
            }
        }
    }

    LibNeoChat.DelegateSizeHelper {
        id: chatBarSizeHelper
        parentItem: root
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: NeoChatConfig.compactLayout ? 100 : 85
        maxWidth: NeoChatConfig.compactLayout ? root.width - Kirigami.Units.largeSpacing * 2 : Kirigami.Units.gridUnit * 60
    }

    Component {
        id: replyPane
        Item {
            implicitWidth: replyComponent.implicitWidth
            implicitHeight: replyComponent.implicitHeight
            ReplyComponent {
                id: replyComponent
                replyContentModel: ContentProvider.contentModelForEvent(root.currentRoom, _private.chatBarCache.replyId, true)
                Message.maxContentWidth: replyLoader.item.width

                // When the user replies to a message and the preview is loaded, make sure the text field is focused again
                Component.onCompleted: textField.forceActiveFocus(Qt.OtherFocusReason)
            }
            QQC2.Button {
                id: cancelButton

                anchors.top: parent.top
                anchors.right: parent.right

                display: QQC2.AbstractButton.IconOnly
                text: i18nc("@action:button", "Cancel reply")
                icon.name: "dialog-close"
                onClicked: {
                    _private.chatBarCache.replyId = "";
                    _private.chatBarCache.attachmentPath = "";
                }
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
    Component {
        id: attachmentPane
        AttachmentPane {
            attachmentPath: _private.chatBarCache.attachmentPath

            onAttachmentCancelled: {
                _private.chatBarCache.attachmentPath = "";
                root.forceActiveFocus();
            }
        }
    }

    QtObject {
        id: _private
        property ChatBarCache chatBarCache
        onChatBarCacheChanged: {
            richEditBar.chatBarCache = chatBarCache
        }

        function postMessage() {
            _private.chatBarCache.postMessage();
            repeatTimer.stop();
            textField.clear();
        }

        function formatText(format, selectionStart, selectionEnd) {
            let index = textField.cursorPosition;

            /*
            * There cannot be white space at the beginning or end of the string for the
            * formatting to work so move the sectionStart and sectionEnd markers past any whitespace.
            */
            let innerText = textField.text.substr(selectionStart, selectionEnd - selectionStart);
            if (innerText.charAt(innerText.length - 1) === " ") {
                let trimmedRightString = innerText.replace(/\s*$/, "");
                let trimDifference = innerText.length - trimmedRightString.length;
                selectionEnd -= trimDifference;
            }
            if (innerText.charAt(0) === " ") {
                let trimmedLeftString = innerText.replace(/^\s*/, "");
                let trimDifference = innerText.length - trimmedLeftString.length;
                selectionStart = selectionStart + trimDifference;
            }
            let startText = textField.text.substr(0, selectionStart);
            // Needs updating with the new selectionStart and selectionEnd with white space trimmed.
            innerText = textField.text.substr(selectionStart, selectionEnd - selectionStart);
            let endText = textField.text.substr(selectionEnd);
            textField.text = "";
            textField.text = startText + format.start + innerText + format.end + format.extra + endText;

            /*
            * Put the cursor where it was when the popup was opened accounting for the
            * new markup.
            *
            * The exception is for a hyperlink where it is placed ready to start typing
            * the url.
            */
            if (format.extra !== "") {
                textField.cursorPosition = selectionEnd + format.start.length + format.end.length;
            } else if (index == selectionStart) {
                textField.cursorPosition = index;
            } else {
                textField.cursorPosition = index + format.start.length + format.end.length;
            }
        }

        function pasteImage() {
            let localPath = Clipboard.saveImage();
            if (localPath.length === 0) {
                return false;
            }
            _private.chatBarCache.attachmentPath = localPath;
            return true;
        }
    }

    CompletionMenu {
        id: completionMenu
        chatDocumentHandler: documentHandler
        connection: root.connection

        x: 1
        y: -height
        width: parent.width - 1
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }
}
