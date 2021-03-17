/* SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.platform 1.0 as Platform
import org.kde.kirigami 2.14 as Kirigami

import org.kde.neochat 1.0

ToolBar {
    id: chatBar
    property string replyEventId: ""
    property string editEventId: ""
    property string inputFieldText: currentRoom ? currentRoom.cachedInput : ""
    property alias textField: inputField
    property alias emojiPaneOpened: emojiButton.checked

    // store each user we autoComplete here, this will be helpful later to generate
    // the matrix.to links.
    // This use an hack to define: https://doc.qt.io/qt-5/qml-var.html#property-value-initialization-semantics
    property var userAutocompleted: ({})

    signal attachTriggered(string localPath)
    signal closeAllTriggered()
    signal inputFieldForceActiveFocusTriggered()
    signal messageSent()
    signal pasteImageTriggered()

    property alias isCompleting: completionMenu.visible

    onInputFieldForceActiveFocusTriggered: inputField.forceActiveFocus()

    position: ToolBar.Footer

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

        ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: inputField.implicitHeight
            // lineSpacing is height+leading, so subtract leading once since leading only exists between lines.
            Layout.maximumHeight: fontMetrics.lineSpacing * 8 - fontMetrics.leading
                                + inputField.topPadding + inputField.bottomPadding

            FontMetrics {
                id: fontMetrics
                font: inputField.font
            }
            TextArea {
                id: inputField
                focus: true
                /* Some QQC2 styles will have their own predefined backgrounds for TextAreas.
                * Make sure there is no background since we are using the ToolBar background.
                *
                * This could cause a problem if the QQC2 style was designed around TextArea
                * background colors being very different from the QPalette::Base color.
                * Luckily, none of the Qt QQC2 styles do that and neither do KDE's QQC2 styles.
                */
                background: null
                leftPadding: mirrored ? 0 : Kirigami.Units.largeSpacing
                rightPadding: !mirrored ? 0 : Kirigami.Units.largeSpacing
                topPadding: 0
                bottomPadding: 0

                property real progress: 0
                property bool autoAppeared: false
                //property int lineHeight: contentHeight / lineCount

                text: inputFieldText
                placeholderText: editEventId.length > 0 ? i18n("Edit Message") : i18n("Write your message...")
                verticalAlignment: TextEdit.AlignVCenter
                horizontalAlignment: TextEdit.AlignLeft
                wrapMode: Text.Wrap

                ChatDocumentHandler {
                    id: documentHandler
                    document: inputField.textDocument
                    cursorPosition: inputField.cursorPosition
                    selectionStart: inputField.selectionStart
                    selectionEnd: inputField.selectionEnd
                    room: currentRoom ?? null
                }

                Timer {
                    id: timeoutTimer
                    repeat: false
                    interval: 2000
                    onTriggered: {
                        repeatTimer.stop()
                        currentRoom.sendTypingNotification(false)
                    }
                }

                Timer {
                    id: repeatTimer
                    repeat: true
                    interval: 5000
                    triggeredOnStart: true
                    onTriggered: currentRoom.sendTypingNotification(true)
                }

                Keys.onReturnPressed: {
                    if (isCompleting) {
                        chatBar.complete();

                        isCompleting = false;
                        return;
                    }
                    if (event.modifiers & Qt.ShiftModifier) {
                        inputField.insert(cursorPosition, "\n")
                    } else {
                        chatBar.postMessage()
                    }
                }

                Keys.onEscapePressed: {
                    closeAllTriggered()
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_PageDown) {
                        switchRoomDown();
                    } else if (event.key === Qt.Key_PageUp) {
                        switchRoomUp();
                    } else if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                        chatBar.pasteImage();
                    }
                }

                Keys.onBacktabPressed: {
                    if (event.modifiers & Qt.ControlModifier) {
                        switchRoomUp();
                        return;
                    }
                    if (isCompleting) {
                        let decrementedIndex = completionMenu.currentIndex - 1
                        // Wrap around to the last item
                        if (decrementedIndex < 0) {
                            decrementedIndex = Math.max(completionMenu.count - 1, 0) // 0 if count == 0
                        }
                        completionMenu.currentIndex = decrementedIndex
                    }
                }

                Keys.onTabPressed: {
                    if (event.modifiers & Qt.ControlModifier) {
                        switchRoomDown();
                        return;
                    }
                    if (!isCompleting) {
                        return;
                    }

                    // TODO detect moved cursor

                    // ignore first time tab was clicked so that user can select
                    // first emoji/user
                    if (autoAppeared === false) {
                        let incrementedIndex = completionMenu.currentIndex + 1;
                        // Wrap around to the first item
                        if (incrementedIndex > completionMenu.count - 1) {
                            incrementedIndex = 0
                        }
                        completionMenu.currentIndex = incrementedIndex;
                    } else {
                        autoAppeared = false;
                    }

                    chatBar.complete();
                }

                onTextChanged: {
                    timeoutTimer.restart()
                    repeatTimer.start()
                    currentRoom.cachedInput = text
                    autoAppeared = false;

                    const completionInfo = documentHandler.getAutocompletionInfo();

                    if (completionInfo.type === ChatDocumentHandler.Ignore) {
                        return;
                    }
                    if (completionInfo.type === ChatDocumentHandler.None) {
                        isCompleting = false;
                        return;
                    }

                    if (completionInfo.type === ChatDocumentHandler.User) {
                        completionMenu.isCompletingEmoji = false
                        completionMenu.model = currentRoom.getUsers(completionInfo.keyword);
                    } else {
                        completionMenu.isCompletingEmoji = true
                        completionMenu.model = completionMenu.emojiModel.filterModel(completionInfo.keyword);
                    }

                    if (completionMenu.model.length === 0) {
                        isCompleting = false;
                        return;
                    }
                    isCompleting = true
                    autoAppeared = true;
                    completionMenu.endPosition = cursorPosition
                }
            }
        }

        Item {
            visible: !isReply && (!hasAttachment || uploadingBusySpinner.running)
            implicitWidth: uploadButton.implicitWidth
            implicitHeight: uploadButton.implicitHeight
            ToolButton {
                id: uploadButton
                anchors.fill: parent
                // Matrix does not allow sending attachments in replies
                visible: !isReply && !hasAttachment && !uploadingBusySpinner.running
                icon.name: "mail-attachment"
                text: i18n("Attach an image or file")
                display: AbstractButton.IconOnly

                onClicked: {
                    if (Clipboard.hasImage) {
                        attachDialog.open()
                    } else {
                        var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)
                        fileDialog.chosen.connect((path) => {
                            if (!path) { return }
                            attachTriggered(path)
                        })
                        fileDialog.open()
                    }
                }

                ToolTip.text: text
                ToolTip.visible: hovered
            }
            BusyIndicator {
                id: uploadingBusySpinner
                anchors.fill: parent
                visible: running
                running: currentRoom && currentRoom.hasFileUploading
            }
        }

        ToolButton {
            id: emojiButton
            icon.name: "preferences-desktop-emoticons"
            text: i18n("Add an Emoji")
            display: AbstractButton.IconOnly
            checkable: true

            ToolTip.text: text
            ToolTip.visible: hovered
        }

        ToolButton {
            id: sendButton
            icon.name: "document-send"
            text: i18n("Send message")
            display: AbstractButton.IconOnly

            onClicked: {
                chatBar.postMessage()
            }

            ToolTip.text: text
            ToolTip.visible: hovered
        }
    }

    Action {
        id: pasteAction
        shortcut: StandardKey.Paste
        onTriggered: {
            if (Clipboard.hasImage) {
                pasteImageTriggered();
            }
            activeFocusItem.paste();
        }
    }

    CompletionMenu {
        id: completionMenu
        width: parent.width
        //height: 80 //Math.min(implicitHeight, delegate.implicitHeight * 6)
        height: implicitHeight
        y: -height - 1
        z: 1
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
        onCompleteTriggered: {
            complete()
            isCompleting = false;
        }
    }

    function pasteImage() {
        let localPath = Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png";
        if (!Clipboard.saveImage(localPath)) {
            return;
        }
        attachTriggered(localPath)
    }

    function postMessage() {
        checkForFancyEffectsReason();
        roomManager.actionsHandler.postMessage(inputField.text.trim(), attachmentPath,
            replyEventId, editEventId, userAutocompleted);
        currentRoom.markAllMessagesAsRead();
        inputField.clear();
        inputField.text = Qt.binding(function() {
            return currentRoom ? currentRoom.cachedInput : "";
        });
        messageSent()
    }

    function complete() {
        documentHandler.replaceAutoComplete(completionMenu.currentDisplayText);
        if (!completionMenu.isCompletingEmoji) {
            userAutocompleted[completionMenu.currentDisplayText] = completionMenu.currentUserId;
        }
    }
}
