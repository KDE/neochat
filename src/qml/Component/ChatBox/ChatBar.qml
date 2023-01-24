// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.18 as Kirigami
import org.kde.neochat 1.0

QQC2.Control {
    id: root

    property alias textField: textField
    property bool isReplying: currentRoom.chatBoxReplyId.length > 0
    property bool isEditing: currentRoom.chatBoxEditId.length > 0
    property bool replyPaneVisible: isReplying || isEditing
    property NeoChatUser replyUser: currentRoom.chatBoxReplyUser ?? currentRoom.chatBoxEditUser
    property bool attachmentPaneVisible: currentRoom.chatBoxAttachmentPath.length > 0

    signal messageSent()

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
            text: i18n("Add an Emoji")
            displayHint: Kirigami.DisplayHint.IconOnly
            checkable: true

            onTriggered: {
                if (emojiDialog.visible) {
                    emojiDialog.close()
                } else {
                    emojiDialog.open()
                }
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
                root.postMessage()
            }

            tooltip: text
        }
    ]

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: QQC2.ScrollView {
        id: chatBarScrollView

        property var textFieldHeight: textField.height

        property var visualLeftPadding: (root.width - chatBoxMaxWidth) / 2 - (root.width > chatBoxMaxWidth ? Kirigami.Units.largeSpacing : 0)
        property var visualRightPadding: (root.width - chatBoxMaxWidth) / 2 + (root.width > chatBoxMaxWidth ? Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing : 0)
        leftPadding: LayoutMirroring.enabled ? visualRightPadding : visualLeftPadding
        rightPadding: LayoutMirroring.enabled ? visualLeftPadding : visualRightPadding

        // HACK: This is to stop the ScrollBar flickering on and off as the height is increased
        QQC2.ScrollBar.vertical.policy: chatBarHeightAnimation.running && implicitHeight <= height ? QQC2.ScrollBar.AlwaysOff : QQC2.ScrollBar.AsNeeded

        Behavior on implicitHeight {
            NumberAnimation {
                id: chatBarHeightAnimation
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.InOutCubic
            }
        }

        QQC2.TextArea{
            id: textField

            topPadding: Kirigami.Units.largeSpacing + (paneLoader.visible ? paneLoader.height : 0)
            bottomPadding: Kirigami.Units.largeSpacing
            leftPadding: LayoutMirroring.enabled ? actionsRow.width : (root.width > chatBoxMaxWidth ? 0 : Kirigami.Units.largeSpacing)
            rightPadding: LayoutMirroring.enabled ? (root.width > chatBoxMaxWidth ? 0 : Kirigami.Units.largeSpacing) : actionsRow.width

            placeholderText: readOnly ? i18n("This room is encrypted. Build libQuotient with encryption enabled to send encrypted messages.") : currentRoom.chatBoxEditId.length > 0 ? i18n("Edit Message") : currentRoom.usesEncryption ? i18n("Send an encrypted message…") : currentRoom.chatBoxAttachmentPath.length > 0 ? i18n("Set an attachment caption...") : i18n("Send a message…")
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

            Keys.onEnterPressed: {
                if (completionMenu.visible) {
                    completionMenu.complete()
                } else if (event.modifiers & Qt.ShiftModifier) {
                    textField.insert(cursorPosition, "\n")
                } else {
                    chatBar.postMessage();
                }
            }
            Keys.onReturnPressed: {
                if (completionMenu.visible) {
                    completionMenu.complete()
                } else if (event.modifiers & Qt.ShiftModifier) {
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
                    let replyEvent = messageEventModel.getLatestMessageFromIndex(0)
                    if (replyEvent && replyEvent["event_id"]) {
                        currentRoom.chatBoxReplyId = replyEvent["event_id"]
                    }
                } else if (event.key === Qt.Key_Up && textField.text.length === 0) {
                    let editEvent = messageEventModel.getLastLocalUserMessageEventId()
                    if (editEvent) {
                        currentRoom.chatBoxEditId = editEvent["event_id"]
                    }
                } else if (event.key === Qt.Key_Up && completionMenu.visible) {
                    completionMenu.decrementIndex()
                } else if (event.key === Qt.Key_Down && completionMenu.visible) {
                    completionMenu.incrementIndex()
                } else if (event.key === Qt.Key_Backspace && textField.text.length <= 1) {
                    currentRoom.sendTypingNotification(false)
                    repeatTimer.stop()
                }
            }

            Loader {
                id: paneLoader

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: root.width > chatBoxMaxWidth ? 0 : Kirigami.Units.largeSpacing
                anchors.right: parent.right
                anchors.rightMargin: root.width > chatBoxMaxWidth ? 0 : (chatBarScrollView.QQC2.ScrollBar.vertical.visible ? Kirigami.Units.largeSpacing * 3.5 : Kirigami.Units.largeSpacing)

                active: visible
                visible: root.replyPaneVisible || root.attachmentPaneVisible
                sourceComponent: root.replyPaneVisible ? replyPane : attachmentPane
            }
            Component {
                id: replyPane
                ReplyPane {
                    userName: root.replyUser ? root.replyUser.displayName : ""
                    userColor: root.replyUser ? root.replyUser.color : ""
                    userAvatar: root.replyUser ? "image://mxc/" + currentRoom.getUser(root.replyUser.id).avatarMediaId : ""
                    isReply: root.isReplying
                    text: isEditing ? currentRoom.chatBoxEditMessage : currentRoom.chatBoxReplyMessage
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
        anchors.rightMargin: (root.width - chatBoxMaxWidth) / 2 + Kirigami.Units.largeSpacing + (chatBarScrollView.QQC2.ScrollBar.vertical.visible && !(root.width > chatBoxMaxWidth) ? Kirigami.Units.largeSpacing * 2.5 : 0)

        visible: root.replyPaneVisible
        display: QQC2.AbstractButton.IconOnly
        action: Kirigami.Action {
            text: root.isReplying ? i18nc("@action:button", "Cancel reply") : i18nc("@action:button", "Cancel edit")
            icon.name: "dialog-close"
            onTriggered: {
                currentRoom.chatBoxReplyId = "";
                currentRoom.chatBoxEditId = "";
                currentRoom.chatBoxAttachmentPath = "";
                root.forceActiveFocus()
            }
            shortcut: "Escape"
        }
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
    }
    Row {
        id: actionsRow
        padding: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing
        anchors.right: parent.right
        property var requiredMargin: (root.width - chatBoxMaxWidth) / 2 + Kirigami.Units.largeSpacing + (chatBarScrollView.QQC2.ScrollBar.vertical.visible && !(root.width > chatBoxMaxWidth) ? Kirigami.Units.largeSpacing * 2.5 : 0)
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

                QQC2.BusyIndicator {
                    anchors.fill: parent
                    leftPadding: 0
                    rightPadding: 0
                    topPadding: 0
                    bottomPadding: 0
                    visible: running
                    running: modelData.isBusy
                }
            }
        }
    }

    EmojiDialog {
        id: emojiDialog
        x: parent.width - implicitWidth
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

    Connections {
        target: currentRoom
        function onChatBoxEditIdChanged() {
            if (currentRoom.chatBoxEditMessage.length > 0) {
                textField.text = currentRoom.chatBoxEditMessage
            }
        }
    }

    ChatDocumentHandler {
        id: documentHandler
        document: textField.textDocument
        cursorPosition: textField.cursorPosition
        selectionStart: textField.selectionStart
        selectionEnd: textField.selectionEnd
        Component.onCompleted: {
            RoomManager.chatDocumentHandler = documentHandler;
        }
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
        actionsHandler.handleMessage();
        repeatTimer.stop()
        currentRoom.markAllMessagesAsRead();
        textField.clear();
        currentRoom.chatBoxReplyId = "";
        currentRoom.chatBoxEditId = "";
        messageSent()
    }
}
