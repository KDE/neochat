// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Window 2.15

import org.kde.kirigami 2.18 as Kirigami
import org.kde.neochat 1.0

/**
 * @brief The component which handles the message sending.
 *
 * The ChatBox deals with laying out the visual elements with the ChatBar handling
 * the core functionality of displaying the current message composition before sending.
 *
 * This includes support for the following message types:
 *  - text
 *  - media (video, image, file)
 *  - emojis/stickers
 *  - location
 *
 * In addition, when replying, this component supports showing the message that is being
 * replied to.
 *
 * @note There is no edit functionality here this, is handled inline by the timeline
 *       text delegate.
 *
 * @sa ChatBox
 */
QQC2.Control {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    property var currentRoom

    /**
     * @brief The QQC2.TextArea object.
     *
     * @sa QQC2.TextArea
     */
    property alias textField: textField

    /**
     * @brief The list of actions in the ChatBar.
     *
     * Each of these will be visualised in the ChatBar so new actions can be added
     * by appending to this list.
     */
    property list<Kirigami.Action> actions : [
        Kirigami.Action {
            id: attachmentAction

            property bool isBusy: currentRoom && currentRoom.hasFileUploading

            // Matrix does not allow sending attachments in replies
            visible: currentRoom.chatBoxReplyId.length === 0  && currentRoom.chatBoxAttachmentPath.length === 0
            icon.name: "mail-attachment"
            text: i18n("Attach an image or file")
            displayHint: Kirigami.DisplayHint.IconOnly

            onTriggered: {
                if (Clipboard.hasImage) {
                    attachDialog.open()
                } else {
                    var fileDialog = openFileDialog.createObject(QQC2.ApplicationWindow.overlay)
                    fileDialog.chosen.connect((path) => {
                        if (!path) {
                            return;
                        }
                        currentRoom.chatBoxAttachmentPath = path;
                    })
                    fileDialog.open()
                }
            }

            tooltip: text
        },
        Kirigami.Action {
            id: emojiAction

            property bool isBusy: false

            icon.name: "smiley"
            text: i18n("Emojis & Stickers")
            displayHint: Kirigami.DisplayHint.IconOnly
            checkable: true

            onTriggered: {
                if (emojiDialog.visible) {
                    emojiDialog.close()
                } else {
                    emojiDialog.open()
                }
            }
        },
        Kirigami.Action {
            id: mapButton
            icon.name: "globe"
            property bool isBusy: false
            text: i18n("Send a Location")
            displayHint: QQC2.AbstractButton.IconOnly

            onTriggered: {
                locationChooserComponent.createObject(QQC2.ApplicationWindow.overlay, {room: currentRoom}).open()
            }
        },
        Kirigami.Action {
            id: sendAction

            property bool isBusy: false

            icon.name: "document-send"
            text: i18n("Send message")
            displayHint: Kirigami.DisplayHint.IconOnly
            checkable: true

            onTriggered: {
                root.postMessage()
            }

            tooltip: text
        }
    ]

    /**
     * @brief A message has been sent from the chat bar.
     */
    signal messageSent()

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: QQC2.ScrollView {
        id: chatBarScrollView

        property var textFieldHeight: textField.height

        // HACK: This is to stop the ScrollBar flickering on and off as the height is increased
        QQC2.ScrollBar.vertical.policy: chatBarHeightAnimation.running && implicitHeight <= height ? QQC2.ScrollBar.AlwaysOff : QQC2.ScrollBar.AsNeeded

        Behavior on implicitHeight {
            NumberAnimation {
                id: chatBarHeightAnimation
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.InOutCubic
            }
        }

        QQC2.TextArea {
            id: textField

            x: Math.round((root.width - chatBarSizeHelper.currentWidth) / 2) - (root.width > chatBarSizeHelper.currentWidth + Kirigami.Units.largeSpacing * 2.5 ? Kirigami.Units.largeSpacing * 1.5 : 0)
            topPadding: Kirigami.Units.largeSpacing + (paneLoader.visible ? paneLoader.height : 0)
            bottomPadding: Kirigami.Units.largeSpacing
            leftPadding: LayoutMirroring.enabled ? actionsRow.width : Kirigami.Units.largeSpacing
            rightPadding: LayoutMirroring.enabled ? Kirigami.Units.largeSpacing : actionsRow.width + x * 2 + Kirigami.Units.largeSpacing * 2

            placeholderText: readOnly ? i18n("This room is encrypted. Build libQuotient with encryption enabled to send encrypted messages.") : currentRoom.usesEncryption ? i18n("Send an encrypted message…") : currentRoom.chatBoxAttachmentPath.length > 0 ? i18n("Set an attachment caption...") : i18n("Send a message…")
            verticalAlignment: TextEdit.AlignVCenter
            wrapMode: Text.Wrap
            readOnly: (currentRoom.usesEncryption && !Controller.encryptionSupported)

            Timer {
                id: repeatTimer
                interval: 5000
            }

            onTextChanged: {
                if (!repeatTimer.running && Config.typingNotifications) {
                    var textExists = text.length > 0
                    currentRoom.sendTypingNotification(textExists)
                    textExists ? repeatTimer.start() : repeatTimer.stop()
                }
                currentRoom.chatBoxText = text
            }
            onCursorRectangleChanged: chatBarScrollView.ensureVisible(cursorRectangle)
            onSelectedTextChanged: {
                if (selectedText.length > 0) {
                    quickFormatBar.selectionStart = selectionStart
                    quickFormatBar.selectionEnd = selectionEnd
                    quickFormatBar.open()
                }
            }

            QuickFormatBar {
                id: quickFormatBar

                x: textField.cursorRectangle.x
                y: textField.cursorRectangle.y - height

                onFormattingSelected: chatBar.formatText(format, selectionStart, selectionEnd)
            }

            Keys.onDeletePressed: {
                if (selectedText.length > 0) {
                    remove(selectionStart, selectionEnd)
                } else {
                    remove(cursorPosition, cursorPosition + 1)
                }
                if (textField.text == selectedText || textField.text.length <= 1) {
                    currentRoom.sendTypingNotification(false)
                    repeatTimer.stop()
                }
                if (quickFormatBar.visible) {
                    quickFormatBar.close()
                }
            }
            Keys.onEnterPressed: {
                if (completionMenu.visible) {
                    completionMenu.complete()
                } else if (event.modifiers & Qt.ShiftModifier || Kirigami.Settings.isMobile) {
                    textField.insert(cursorPosition, "\n")
                } else {
                    chatBar.postMessage();
                }
            }
            Keys.onReturnPressed: {
                if (completionMenu.visible) {
                    completionMenu.complete()
                } else if (event.modifiers & Qt.ShiftModifier || Kirigami.Settings.isMobile) {
                    textField.insert(cursorPosition, "\n")
                } else {
                    chatBar.postMessage();
                }
            }
            Keys.onTabPressed: {
                if (completionMenu.visible) {
                    completionMenu.complete()
                }
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                    chatBar.pasteImage();
                } else if (event.key === Qt.Key_Up && event.modifiers & Qt.ControlModifier) {
                    currentRoom.replyLastMessage();
                } else if (event.key === Qt.Key_Up && textField.text.length === 0) {
                    currentRoom.editLastMessage();
                } else if (event.key === Qt.Key_Up && completionMenu.visible) {
                    completionMenu.decrementIndex()
                } else if (event.key === Qt.Key_Down && completionMenu.visible) {
                    completionMenu.incrementIndex()
                } else if (event.key === Qt.Key_Backspace) {
                    if (textField.text == selectedText || textField.text.length <= 1) {
                        currentRoom.sendTypingNotification(false)
                        repeatTimer.stop()
                    }
                    if (quickFormatBar.visible && selectedText.length > 0) {
                        quickFormatBar.close()
                    }
                }
            }
            Keys.onShortcutOverride: {
                // Accept the event only when there was something to cancel. Otherwise, let the event go to the RoomPage.
                if (cancelButton.visible && event.key === Qt.Key_Escape) {
                    cancelButton.action.trigger();
                    event.accepted = true;
                }
            }

            Loader {
                id: paneLoader

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Kirigami.Units.largeSpacing
                anchors.right: parent.right
                anchors.rightMargin: root.width > chatBarSizeHelper.currentWidth ? 0 : (chatBarScrollView.QQC2.ScrollBar.vertical.visible ? Kirigami.Units.largeSpacing * 3.5 : Kirigami.Units.largeSpacing)

                active: visible
                visible: root.currentRoom.chatBoxReplyId.length > 0 || root.currentRoom.chatBoxAttachmentPath.length > 0
                sourceComponent: root.currentRoom.chatBoxReplyId.length > 0 ? replyPane : attachmentPane
            }
            Component {
                id: replyPane
                ReplyPane {
                    userName: currentRoom.chatBoxReplyUser.displayName
                    userColor: currentRoom.chatBoxReplyUser.color
                    userAvatar: currentRoom.chatBoxReplyUser.avatarSource
                    text: currentRoom.chatBoxReplyMessage
                }
            }
            Component {
                id: attachmentPane
                AttachmentPane {
                    attachmentPath: currentRoom.chatBoxAttachmentPath

                    onAttachmentCancelled: {
                        currentRoom.chatBoxAttachmentPath = "";
                        root.forceActiveFocus()
                    }
                }
            }

            background: MouseArea {
                acceptedButtons: Qt.NoButton
                cursorShape: Qt.IBeamCursor
                z: 1
            }
        }

        /**
         * Because of the paneLoader we have to manage the scroll
         * position manually or it doesn't keep the cursor visible properly all the time.
         */
        function ensureVisible(r) {
            // Find the child that is the Flickable created by ScrollView.
            let flickable = undefined;
            for (var index in children) {
                if (children[index] instanceof Flickable) {
                    flickable = children[index];
                }
            }

            if (flickable) {
                if (flickable.contentX >= r.x) {
                    flickable.contentX = r.x;
                } else if (flickable.contentX + width <= r.x + r.width) {
                    flickable.contentX = r.x + r.width - width;
                } if (flickable.contentY >= r.y) {
                    flickable.contentY = r.y;
                } else if (flickable.contentY + height <= r.y + r.height) {
                    flickable.contentY = r.y + r.height - height + textField.bottomPadding;
                }
            }
        }
    }

    QQC2.ToolButton {
        id: cancelButton
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: (root.width - chatBarSizeHelper.currentWidth) / 2 + Kirigami.Units.largeSpacing + (chatBarScrollView.QQC2.ScrollBar.vertical.visible && !(root.width > chatBarSizeHelper.currentWidth) ? Kirigami.Units.largeSpacing * 2.5 : 0)

        visible: root.currentRoom.chatBoxReplyId.length > 0
        display: QQC2.AbstractButton.IconOnly
        action: Kirigami.Action {
            text: i18nc("@action:button", "Cancel reply")
            icon.name: "dialog-close"
            onTriggered: {
                currentRoom.chatBoxReplyId = "";
                currentRoom.chatBoxAttachmentPath = "";
                root.forceActiveFocus()
            }
        }
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
    }
    Row {
        id: actionsRow
        padding: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing
        anchors.right: parent.right
        property var requiredMargin: (root.width - chatBarSizeHelper.currentWidth) / 2 + Kirigami.Units.largeSpacing + (chatBarScrollView.QQC2.ScrollBar.vertical.visible && !(root.width > chatBarSizeHelper.currentWidth) ? Kirigami.Units.largeSpacing * 2.5 : 0)
        anchors.leftMargin: layoutDirection === Qt.RightToLeft ? requiredMargin : 0
        anchors.rightMargin: layoutDirection === Qt.RightToLeft ? 0 : requiredMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Kirigami.Units.largeSpacing - 2

        Repeater {
            model: root.actions
            Kirigami.Icon {
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                implicitHeight: Kirigami.Units.iconSizes.smallMedium

                source: modelData.isBusy ? "" : (modelData.icon.name.length > 0 ? modelData.icon.name : modelData.icon.source)
                active: actionArea.containsPress
                visible: modelData.visible
                enabled: modelData.enabled
                MouseArea {
                    id: actionArea
                    anchors.fill: parent
                    onClicked: modelData.trigger()
                    cursorShape: Qt.PointingHandCursor
                }

                QQC2.ToolTip.visible: modelData.tooltip !== "" && hoverHandler.hovered
                QQC2.ToolTip.text: modelData.tooltip
                HoverHandler { id: hoverHandler }

                PieProgressBar {
                    visible: modelData.isBusy
                    progress: currentRoom.fileUploadingProgress
                }
            }
        }
    }

    EmojiDialog {
        id: emojiDialog
        x: parent.width - width
        y: -implicitHeight // - Kirigami.Units.smallSpacing

        modal: false
        includeCustom: true
        closeOnChosen: false

        onChosen: insertText(emoji)
        onClosed: if (emojiButton.checked) emojiButton.checked = false
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    CompletionMenu {
        id: completionMenu
        height: implicitHeight
        y: -height - 5
        z: 1
        chatDocumentHandler: documentHandler
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    ChatDocumentHandler {
        id: documentHandler
        document: textField.textDocument
        cursorPosition: textField.cursorPosition
        selectionStart: textField.selectionStart
        selectionEnd: textField.selectionEnd
        mentionColor: Kirigami.Theme.linkColor
        errorColor: Kirigami.Theme.negativeTextColor
        Component.onCompleted: {
            RoomManager.chatDocumentHandler = documentHandler;
        }
    }

    DelegateSizeHelper {
        id: chatBarSizeHelper
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: Config.compactLayout ? 100 : 85
        maxWidth: Config.compactLayout ? -1 : Kirigami.Units.gridUnit * 60

        parentWidth: root.width
    }

    function forceActiveFocus() {
        textField.forceActiveFocus();
        // set the cursor to the end of the text
        textField.cursorPosition = textField.length;
    }

    function insertText(text) {
        let initialCursorPosition = textField.cursorPosition;

        textField.text = textField.text.substr(0, initialCursorPosition) + text + textField.text.substr(initialCursorPosition)
        textField.cursorPosition = initialCursorPosition + text.length
    }

    function pasteImage() {
        let localPath = Clipboard.saveImage();
        if (localPath.length === 0) {
            return;
        }
        currentRoom.chatBoxAttachmentPath = localPath
    }

    function postMessage() {
        actionsHandler.handleNewMessage();
        repeatTimer.stop()
        currentRoom.markAllMessagesAsRead();
        textField.clear();
        currentRoom.chatBoxReplyId = "";
        messageSent()
    }

    function formatText(format, selectionStart, selectionEnd) {
        let index = textField.cursorPosition;

        /*
         * There cannot be white space at the beginning or end of the string for the
         * formatting to work so move the sectionStart and sectionEnd markers past any whitespace.
         */
        let innerText = textField.text.substr(selectionStart, selectionEnd - selectionStart);
        if (innerText.charAt(innerText.length - 1) === " ") {
            let trimmedRightString = innerText.replace(/\s*$/,"");
            let trimDifference = innerText.length - trimmedRightString.length;
            selectionEnd -= trimDifference;
        }
        if (innerText.charAt(0) === " ") {
            let trimmedLeftString = innerText.replace(/^\s*/,"");
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

    Component {
        id: locationChooserComponent
        LocationChooser {}
    }

    Component {
        id: openFileDialog

        OpenFileDialog {
            parentWindow: Window.window
        }
    }
}
