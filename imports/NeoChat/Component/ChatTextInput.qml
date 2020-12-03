/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Component.Emoji 1.0
import NeoChat.Dialog 1.0
import NeoChat.Effect 1.0

import org.kde.neochat 1.0

ToolBar {
    id: root

    property alias isReply: replyItem.visible
    property bool isReaction: false
    property var replyUser
    property string replyEventID
    property string replyContent

    property alias isAutoCompleting: autoCompleteListView.visible
    property var autoCompleteModel
    property int autoCompleteBeginPosition
    property int autoCompleteEndPosition

    property bool hasAttachment: false
    property url attachmentPath

    position: ToolBar.Footer

    function addText(text) {
        inputField.insert(inputField.length, text)
    }
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    contentItem: ColumnLayout {
        id: layout
        spacing: 0
        EmojiPicker {
            id: emojiPicker

            Layout.fillWidth: true

            visible: false

            textArea: inputField
            emojiModel: EmojiModel { id: emojiModel }
            onChosen: {
                textArea.insert(textArea.cursorPosition, emoji);
                textArea.forceActiveFocus();
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 8

            id: replyItem

            visible: false

            spacing: 8

            Control {
                Layout.alignment: Qt.AlignTop

                padding: 4

                contentItem: RowLayout {
                    Kirigami.Avatar {
                        Layout.preferredWidth: Kirigami.Units.gridUnit
                        Layout.preferredHeight: Kirigami.Units.gridUnit

                        source: replyUser ? "image://mxc/" + replyUser.avatarMediaId: ""
                        name: replyUser ? replyUser.displayName : i18n("No name")
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: replyUser ? replyUser.displayName : i18n("No name")
                        rightPadding: 8
                    }
                }
            }

            TextEdit {
                Layout.fillWidth: true

                text: "<style>a{color: " + color + ";} .user-pill{}</style>" + replyContent
                color: Kirigami.Theme.textColor

                selectByMouse: true
                readOnly: true
                wrapMode: Label.Wrap
                selectedTextColor: "white"
                textFormat: Text.RichText
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.margins: 8

            id: autoCompleteListView

            visible: false

            model: autoCompleteModel

            clip: true
            spacing: 4
            orientation: ListView.Horizontal
            highlightFollowsCurrentItem: true
            keyNavigationWraps: true

            delegate: Control {
                property string autoCompleteText: modelData.displayName ? ("<a href=\"https://matrix.to/#/" + modelData.id + "\">" + modelData.displayName + "</a>:") : modelData.unicode
                property string displayText: modelData.displayName ?? modelData.unicode
                property bool isEmoji: modelData.unicode != null
                readonly property bool highlighted: autoCompleteListView.currentIndex == index

                padding: Kirigami.Units.smallSpacing

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Label {
                        width: Kirigami.Units.gridUnit
                        height: Kirigami.Units.gridUnit
                        visible: isEmoji
                        text: autoCompleteText
                        font.family: "Emoji"
                        font.pointSize: 20
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Kirigami.Avatar {
                        Layout.preferredWidth: Kirigami.Units.gridUnit
                        Layout.preferredHeight: Kirigami.Units.gridUnit
                        source: modelData.avatarMediaId ? "image://mxc/" + modelData.avatarMediaId : ""
                        color: modelData.color ? Qt.darker(modelData.color, 1.1) : null
                        visible: !isEmoji
                    }
                    Label {
                        Layout.fillHeight: true

                        visible: !isEmoji
                        text: displayText
                        color: highlighted ? Kirigami.Theme.highlightTextColor : Kirigami.Theme.textColor
                        font.underline: highlighted
                        verticalAlignment: Text.AlignVCenter
                        rightPadding: Kirigami.Units.largeSpacing
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        autoCompleteListView.currentIndex = index
                        documentHandler.replaceAutoComplete(autoCompleteText)
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            visible: emojiPicker.visible || replyItem.visible || autoCompleteListView.visible
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 0 //Kirigami.Units.smallSpacing

            Button {
                id: cancelReplyButton

                visible: isReply

                icon.name: "dialog-cancel"

                onClicked: clearReply()
            }

            Control {
                Layout.margins: 6
                Layout.preferredHeight: 36
                Layout.alignment: Qt.AlignVCenter

                visible: hasAttachment

                rightPadding: 8

                contentItem: RowLayout {
                    spacing: 0

                    ToolButton {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        id: cancelAttachmentButton

                        icon.name: "dialog-cancel"

                        onClicked: {
                            hasAttachment = false;
                            attachmentPath = "";
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter

                        text: attachmentPath !== "" ? attachmentPath.toString().substring(attachmentPath.toString().lastIndexOf('/') + 1, attachmentPath.length) : ""
                    }
                }
            }

            TextArea {
                id: inputField
                property real progress: 0
                property bool autoAppeared: false

                ChatDocumentHandler {
                    id: documentHandler
                    document: inputField.textDocument
                    cursorPosition: inputField.cursorPosition
                    selectionStart: inputField.selectionStart
                    selectionEnd: inputField.selectionEnd
                    room: currentRoom ?? null
                }


                Layout.fillWidth: true


                wrapMode: Text.Wrap
                textFormat: TextEdit.RichText
                placeholderText: i18n("Write your message...")
                topPadding: 0
                bottomPadding: 0
                leftPadding: Kirigami.Units.smallSpacing
                selectByMouse: true
                verticalAlignment: TextEdit.AlignVCenter

                text: currentRoom != null ? currentRoom.cachedInput : ""

                background: Item {}

                Rectangle {
                    width: currentRoom && currentRoom.hasFileUploading ? parent.width * currentRoom.fileUploadingProgress / 100 : 0
                    height: parent.height

                    opacity: 0.2
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
                    if (isAutoCompleting) {
                        documentHandler.replaceAutoComplete(autoCompleteListView.currentItem.autoCompleteText)
                        isAutoCompleting = false;
                        return;
                    }
                    if (event.modifiers & Qt.ShiftModifier) {
                        insert(cursorPosition, "<br />")
                    } else {
                        postMessage(text)
                        text = ""
                        clearReply()
                        closeAll()
                    }
                }

                Keys.onEscapePressed: closeAll()

                Keys.onPressed: {
                    if (event.key === Qt.Key_PageDown) {
                        switchRoomDown();
                    } else if (event.key === Qt.Key_PageUp) {
                        switchRoomUp();
                    }
                }

                Keys.onBacktabPressed: {
                    if (event.modifiers & Qt.ControlModifier) {
                        switchRoomUp();
                        return;
                    }
                    if (isAutoCompleting) {
                        autoCompleteListView.decrementCurrentIndex();
                    }
                }

                Keys.onTabPressed: {
                    if (event.modifiers & Qt.ControlModifier) {
                        switchRoomDown();
                        return;
                    }
                    if (!isAutoCompleting) {
                        return;
                    }

                    // TODO detect moved cursor

                    // ignore first time tab was clicked so that user can select
                    // first emoji/user
                    if (autoAppeared === false) {
                        autoCompleteListView.incrementCurrentIndex()
                    } else {
                        autoAppeared = false;
                    }

                    documentHandler.replaceAutoComplete(autoCompleteListView.currentItem.autoCompleteText)
                }

                onTextChanged: {
                    timeoutTimer.restart()
                    repeatTimer.start()
                    currentRoom.cachedInput = text
                    autoAppeared = false;

                    const autocompletionInfo = documentHandler.getAutocompletionInfo();

                    if (autocompletionInfo.type === ChatDocumentHandler.Ignore) {
                        return;
                    }
                    if (autocompletionInfo.type === ChatDocumentHandler.None) {
                        isAutoCompleting = false;
                        autoCompleteListView.currentIndex = 0;
                        return;
                    }

                    if (autocompletionInfo.type === ChatDocumentHandler.User) {
                        autoCompleteModel = currentRoom.getUsers(autocompletionInfo.keyword);
                    } else {
                        autoCompleteModel = emojiModel.filterModel(autocompletionInfo.keyword);
                    }

                    if (autoCompleteModel.length === 0) {
                        isAutoCompleting = false;
                        autoCompleteListView.currentIndex = 0;
                        return;
                    }
                    isAutoCompleting = true
                    autoAppeared = true;
                    autoCompleteEndPosition = cursorPosition
                }

                function postMessage() {
                    documentHandler.postMessage(attachmentPath, replyEventID);
                    clearAttachment();
		    currentRoom.markAllMessagesAsRead();
                    clear();
                }
            }


            ToolButton {
                id: emojiButton
                icon.name: "preferences-desktop-emoticons"
                icon.color: "transparent"

                checkable: true
                checked: emojiPicker.visible
                onToggled: emojiPicker.visible = !emojiPicker.visible

                ToolTip {
                    text: i18n("Add an Emoji")
                }
            }

            ToolButton {
                id: uploadButton

                visible: !isReply && !hasAttachment

                icon.name: "mail-attachment"

                onClicked: {
                    if (Clipboard.hasImage) {
                        attachDialog.open()
                    } else {
                        var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            root.attach(path)
                        })

                        fileDialog.open()
                    }
                }

                ToolTip {
                    text: i18n("Attach an image or file")
                }

                BusyIndicator {
                    anchors.fill: parent

                    running: currentRoom && currentRoom.hasFileUploading
                }
            }

            ToolButton {
                icon.name: "document-send"
                icon.color: "transparent"

                onClicked: {
                    inputField.postMessage(inputField.text)
                    inputField.text = ""
                    root.clearReply()
                    root.closeAll()
                }

                ToolTip {
                    text: i18n("Send message")
                }
            }
        }
    }

    background: Rectangle {
        implicitHeight: 40
        color: Kirigami.Theme.backgroundColor
        Kirigami.Separator {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
        }
    }


    function insert(str) {
        inputField.insert(inputField.cursorPosition, str)
    }

    function clear() {
        inputField.clear()
    }

    function clearReply() {
        isReply = false
        replyUser = null;
        replyContent = "";
        replyEventID = ""
    }

    function focus() {
        inputField.forceActiveFocus()
    }

    function closeAll() {
        replyItem.visible = false
        autoCompleteListView.visible = false
        emojiPicker.visible = false
    }

    function attach(localPath) {
        hasAttachment = true
        attachmentPath = localPath
    }

    function clearAttachment() {
        hasAttachment = false
        attachmentPath = ""
    }
}
