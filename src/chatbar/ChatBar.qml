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

    /**
     * @brief The list of actions in the ChatBar.
     *
     * Each of these will be visualised in the ChatBar so new actions can be added
     * by appending to this list.
     */
    property list<Kirigami.Action> actions: [
        Kirigami.Action {
            id: attachmentAction

            property bool isBusy: root.currentRoom && root.currentRoom.hasFileUploading

            // Matrix does not allow sending attachments in replies
            visible: _private.chatBarCache.replyId.length === 0 && _private.chatBarCache.attachmentPath.length === 0
            icon.name: "mail-attachment"
            text: i18n("Attach an image or file")
            displayHint: Kirigami.DisplayHint.IconOnly

            onTriggered: {
                let dialog = (Clipboard.hasImage ? attachDialog : openFileDialog).createObject(applicationWindow().overlay);
                dialog.chosen.connect(path => _private.chatBarCache.attachmentPath = path);
                dialog.open();
            }

            tooltip: text
        },
        Kirigami.Action {
            id: emojiAction

            property bool isBusy: false

            visible: !Kirigami.Settings.isMobile
            icon.name: "smiley"
            text: i18n("Emojis & Stickers")
            displayHint: Kirigami.DisplayHint.IconOnly
            checkable: true

            onTriggered: {
                if (emojiDialog.visible) {
                    emojiDialog.close();
                } else {
                    emojiDialog.open();
                }
            }
            tooltip: text
        },
        Kirigami.Action {
            id: mapButton
            icon.name: "mark-location-symbolic"
            property bool isBusy: false
            text: i18n("Send a Location")
            displayHint: QQC2.AbstractButton.IconOnly

            onTriggered: {
                locationChooser.createObject(QQC2.Overlay.overlay, {
                    room: root.currentRoom
                }).open();
            }
            tooltip: text
        },
        Kirigami.Action {
            id: pollButton
            icon.name: "amarok_playcount"
            property bool isBusy: false
            text: i18nc("@action:button", "Create a Poll")
            displayHint: QQC2.AbstractButton.IconOnly

            onTriggered: {
                newPollDialog.createObject(QQC2.Overlay.overlay, {
                    room: root.currentRoom
                }).open();
            }
            tooltip: text
        },
        Kirigami.Action {
            id: sendAction

            property bool isBusy: false

            icon.name: "document-send"
            text: i18n("Send message")
            displayHint: Kirigami.DisplayHint.IconOnly
            checkable: true

            onTriggered: {
                _private.postMessage();
            }

            tooltip: text
        }
    ]

    /**
     * @brief A message has been sent from the chat bar.
     */
    signal messageSent

    spacing: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Separator {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
        }
    }

    leftPadding: rightPadding
    rightPadding: (root.width - chatBarSizeHelper.availableWidth) / 2
    topPadding: 0
    bottomPadding: 0

    contentItem: ColumnLayout {
        spacing: 0
        Item {
            // Required to adjust for the top separator
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }
        Loader {
            id: replyLoader

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.preferredHeight: active ? item.implicitHeight : 0

            active: visible
            visible: root.currentRoom.mainCache.replyId.length > 0
            sourceComponent: replyPane
        }
        Loader {
            id: attachLoader

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.preferredHeight: active ? item.implicitHeight : 0

            active: visible
            visible: root.currentRoom.mainCache.attachmentPath.length > 0
            sourceComponent: attachmentPane
        }
        RowLayout {
            QQC2.ScrollView {
                id: chatBarScrollView
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.maximumHeight: Kirigami.Units.gridUnit * 8
                Layout.minimumHeight: Kirigami.Units.gridUnit * 3

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

                    placeholderText: root.currentRoom.usesEncryption ? i18n("Send an encrypted message…") : root.currentRoom.mainCache.attachmentPath.length > 0 ? i18n("Set an attachment caption…") : i18n("Send a message…")
                    verticalAlignment: TextEdit.AlignVCenter
                    wrapMode: TextEdit.Wrap

                    Accessible.description: placeholderText

                    Kirigami.SpellCheck.enabled: false

                    Timer {
                        id: repeatTimer
                        interval: 5000
                    }

                    onTextChanged: {
                        if (!repeatTimer.running && NeoChatConfig.typingNotifications) {
                            var textExists = text.length > 0;
                            root.currentRoom.sendTypingNotification(textExists);
                            textExists ? repeatTimer.start() : repeatTimer.stop();
                        }
                        _private.chatBarCache.text = text;
                    }
                    onSelectedTextChanged: {
                        if (selectedText.length > 0) {
                            quickFormatBar.selectionStart = selectionStart;
                            quickFormatBar.selectionEnd = selectionEnd;
                            quickFormatBar.open();
                        }
                    }

                    QuickFormatBar {
                        id: quickFormatBar

                        x: textField.cursorRectangle.x
                        y: textField.cursorRectangle.y - height

                        onFormattingSelected: _private.formatText(format, selectionStart, selectionEnd)
                    }

                    Keys.onEnterPressed: event => {
                        const controlIsPressed = event.modifiers & Qt.ControlModifier;
                        if (completionMenu.visible) {
                            completionMenu.complete();
                        } else if (event.modifiers & Qt.ShiftModifier || Kirigami.Settings.isMobile || NeoChatConfig.sendMessageWith === 1 && !controlIsPressed || NeoChatConfig.sendMessageWith === 0 && controlIsPressed) {
                            textField.insert(cursorPosition, "\n");
                        } else if (NeoChatConfig.sendMessageWith === 0 && !controlIsPressed || NeoChatConfig.sendMessageWith === 1 && controlIsPressed) {
                            _private.postMessage();
                        }
                    }
                    Keys.onReturnPressed: event => {
                        const controlIsPressed = event.modifiers & Qt.ControlModifier;
                        if (completionMenu.visible) {
                            completionMenu.complete();
                        } else if (event.modifiers & Qt.ShiftModifier || Kirigami.Settings.isMobile || NeoChatConfig.sendMessageWith === 1 && !controlIsPressed || NeoChatConfig.sendMessageWith === 0 && controlIsPressed) {
                            textField.insert(cursorPosition, "\n");
                        } else if (NeoChatConfig.sendMessageWith === 0 && !controlIsPressed || NeoChatConfig.sendMessageWith === 1 && controlIsPressed) {
                            _private.postMessage();
                        }
                    }
                    Keys.onTabPressed: {
                        if (completionMenu.visible) {
                            completionMenu.complete();
                        } else {
                            contextDrawer.handle.children[0].forceActiveFocus()
                        }
                    }
                    Keys.onPressed: event => {
                        if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                            event.accepted = _private.pasteImage();
                        } else if (event.key === Qt.Key_Up && event.modifiers & Qt.ControlModifier) {
                            root.currentRoom.replyLastMessage();
                        } else if (event.key === Qt.Key_Up && textField.text.length === 0) {
                            root.currentRoom.editLastMessage();
                        } else if (event.key === Qt.Key_Up && completionMenu.visible) {
                            completionMenu.decrementIndex();
                        } else if (event.key === Qt.Key_Down && completionMenu.visible) {
                            completionMenu.incrementIndex();
                        } else if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                            if (textField.text == selectedText || textField.text.length <= 1) {
                                root.currentRoom.sendTypingNotification(false);
                                repeatTimer.stop();
                            }
                            if (quickFormatBar.visible && selectedText.length > 0) {
                                quickFormatBar.close();
                            }
                        }
                    }
                    Keys.onShortcutOverride: event => {
                        if (completionMenu.visible) {
                            completionMenu.close();
                        } else if ((_private.chatBarCache.isReplying || _private.chatBarCache.attachmentPath.length > 0) && event.key === Qt.Key_Escape) {
                            _private.chatBarCache.attachmentPath = "";
                            _private.chatBarCache.replyId = "";
                            _private.chatBarCache.threadId = "";
                            event.accepted = true;
                        }
                    }

                    background: MouseArea {
                        acceptedButtons: Qt.NoButton
                        cursorShape: Qt.IBeamCursor
                        z: 1
                    }
                }
            }
            RowLayout {
                id: actionsRow
                spacing: 0
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: Kirigami.Units.smallSpacing * 4

                Repeater {
                    model: root.actions
                    delegate: QQC2.ToolButton {
                        icon.name: modelData.isBusy ? "" : (modelData.icon.name.length > 0 ? modelData.icon.name : modelData.icon.source)
                        onClicked: modelData.trigger()

                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: modelData.tooltip
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                        PieProgressBar {
                            visible: modelData.isBusy
                            progress: root.currentRoom.fileUploadingProgress
                        }
                    }
                }
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
                replyEventId: _private.chatBarCache.replyId
                replyAuthor: _private.chatBarCache.relationAuthor
                replyContentModel: ContentProvider.contentModelForEvent(root.currentRoom, _private.chatBarCache.replyId, true)
                Message.maxContentWidth: replyLoader.item.width
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
        onChatBarCacheChanged: documentHandler.chatBarCache = chatBarCache

        function postMessage() {
            _private.chatBarCache.postMessage();
            repeatTimer.stop();
            root.currentRoom.markAllMessagesAsRead();
            textField.clear();
            messageSent();
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

    Component {
        id: openFileDialog

        OpenFileDialog {
            parentWindow: Window.window
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        }
    }

    Component {
        id: attachDialog

        AttachDialog {
            anchors.centerIn: parent
        }
    }

    Component {
        id: locationChooser
        LocationChooser {}
    }

    Component {
        id: newPollDialog
        NewPollDialog {}
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

    EmojiDialog {
        id: emojiDialog

        x: root.width - width
        y: -implicitHeight

        modal: false
        includeCustom: true
        closeOnChosen: false

        currentRoom: root.currentRoom

        onChosen: emoji => insertText(emoji)
        onClosed: if (emojiAction.checked) {
            emojiAction.checked = false;
        }
    }

    function insertText(text) {
        let initialCursorPosition = textField.cursorPosition;
        textField.text = textField.text.substr(0, initialCursorPosition) + text + textField.text.substr(initialCursorPosition);
        textField.cursorPosition = initialCursorPosition + text.length;
    }
}
