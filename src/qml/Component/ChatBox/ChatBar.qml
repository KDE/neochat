// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Window 2.15

import org.kde.kirigami 2.18 as Kirigami
import org.kde.neochat 1.0

QQC2.ToolBar {
    id: chatBar
    property alias inputFieldText: inputField.text
    property alias textField: inputField
    property alias emojiPaneOpened: emojiButton.checked
    property alias cursorPosition: inputField.cursorPosition

    signal closeAllTriggered()
    signal inputFieldForceActiveFocusTriggered()
    signal messageSent()

    onInputFieldForceActiveFocusTriggered: {
        inputField.forceActiveFocus();
        // set the cursor to the end of the text
        inputField.cursorPosition = inputField.length;
    }

    position: QQC2.ToolBar.Footer

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    // Using a custom background because some styles like Material
    // or Fusion might have ugly colors for a TextArea placed inside
    // of a toolbar. ToolBar is otherwise the closest QQC2 component
    // to what we want because of the padding and spacing values.
    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    contentItem: RowLayout {
        spacing: chatBar.spacing

        QQC2.ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: inputField.implicitHeight
            // lineSpacing is height+leading, so subtract leading once since leading only exists between lines.
            Layout.maximumHeight: fontMetrics.lineSpacing * 8 - fontMetrics.leading
                                + inputField.topPadding + inputField.bottomPadding

            // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

            FontMetrics {
                id: fontMetrics
                font: inputField.font
            }

            QQC2.TextArea {
                id: inputField
                focus: true
                /* Some QQC2 styles will have their own predefined backgrounds for TextAreas.
                * Make sure there is no background since we are using the ToolBar background.
                *
                * This could cause a problem if the QQC2 style was designed around TextArea
                * background colors being very different from the QPalette::Base color.
                * Luckily, none of the Qt QQC2 styles do that and neither do KDE's QQC2 styles.
                */
                background: MouseArea {
                    acceptedButtons: Qt.NoButton
                    cursorShape: Qt.IBeamCursor
                    z: 1
                }

                leftPadding: mirrored ? 0 : Kirigami.Units.largeSpacing
                rightPadding: !mirrored ? 0 : Kirigami.Units.largeSpacing
                topPadding: 0
                bottomPadding: 0

                placeholderText: readOnly ? i18n("This room is encrypted. Sending encrypted messages is not yet supported.") : currentRoom.chatBoxEditId.length > 0 ? i18n("Edit Message") : currentRoom.usesEncryption ? i18n("Send an encrypted message…") : i18n("Send a message…")
                verticalAlignment: TextEdit.AlignVCenter
                horizontalAlignment: TextEdit.AlignLeft
                wrapMode: Text.Wrap
                readOnly: currentRoom.usesEncryption && !Controller.encryptionSupported

                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false

                color: Kirigami.Theme.textColor
                selectionColor: Kirigami.Theme.highlightColor
                selectedTextColor: Kirigami.Theme.highlightedTextColor
                hoverEnabled: !Kirigami.Settings.tabletMode

                selectByMouse: !Kirigami.Settings.tabletMode

                Keys.onEnterPressed: {
                    if (completionMenu.visible) {
                        completionMenu.complete()
                    } else if (event.modifiers & Qt.ShiftModifier) {
                        inputField.insert(cursorPosition, "\n")
                    } else {
                        chatBar.postMessage();
                    }
                }
                Keys.onReturnPressed: {
                    if (completionMenu.visible) {
                        completionMenu.complete()
                    } else if (event.modifiers & Qt.ShiftModifier) {
                        inputField.insert(cursorPosition, "\n")
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
                    } else if (event.key === Qt.Key_Up && inputField.text.length === 0) {
                        let editEvent = messageEventModel.getLastLocalUserMessageEventId()
                        if (editEvent) {
                            currentRoom.chatBoxEditId = editEvent["event_id"]
                        }
                    } else if (event.key === Qt.Key_Up && completionMenu.visible) {
                        completionMenu.decrementIndex()
                    } else if (event.key === Qt.Key_Down && completionMenu.visible) {
                        completionMenu.incrementIndex()
                    } else if (event.key === Qt.Key_Backspace && inputField.text.length <= 1) {
                        currentRoom.sendTypingNotification(false)
                        repeatTimer.stop()
                    }
                }

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
            }
        }

        Item {
            visible: currentRoom.chatBoxReplyId.length === 0 && (currentRoom.chatBoxAttachmentPath.length === 0 || uploadingBusySpinner.running)
            implicitWidth: uploadButton.implicitWidth
            implicitHeight: uploadButton.implicitHeight
            QQC2.ToolButton {
                id: uploadButton
                anchors.fill: parent
                // Matrix does not allow sending attachments in replies
                visible: currentRoom.chatBoxReplyId.length === 0  && currentRoom.chatBoxAttachmentPath.length === 0 && !uploadingBusySpinner.running
                icon.name: "mail-attachment"
                text: i18n("Attach an image or file")
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
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

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
            }
            QQC2.BusyIndicator {
                id: uploadingBusySpinner
                anchors.fill: parent
                visible: running
                running: currentRoom && currentRoom.hasFileUploading
            }
        }

        QQC2.ToolButton {
            id: emojiButton
            icon.name: "smiley"
            text: i18n("Add an Emoji")
            display: QQC2.AbstractButton.IconOnly
            checkable: true

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }

        QQC2.ToolButton {
            id: sendButton
            icon.name: "document-send"
            text: i18n("Send message")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                chatBar.postMessage()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }
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
                chatBar.inputFieldText = currentRoom.chatBoxEditMessage
            }
        }
    }

    ChatDocumentHandler {
        id: documentHandler
        document: inputField.textDocument
        cursorPosition: inputField.cursorPosition
        selectionStart: inputField.selectionStart
        selectionEnd: inputField.selectionEnd
        Component.onCompleted: {
            RoomManager.chatDocumentHandler = documentHandler;
        }
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
        inputField.clear();
        currentRoom.chatBoxReplyId = "";
        currentRoom.chatBoxEditId = "";
        messageSent()
    }
}
