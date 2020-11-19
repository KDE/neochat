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

    contentItem: ColumnLayout {
        spacing: 0
        EmojiPicker {
            id: emojiPicker

            Layout.fillWidth: true

            visible: false

            textArea: inputField
            emojiModel: EmojiModel { id: emojiModel }
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
                property string autoCompleteText: modelData.displayName ?? modelData.unicode
                property bool isEmoji: modelData.unicode != null
                readonly property bool highlighted: autoCompleteListView.currentIndex == index

                height: 36
                padding: 6

                background: Rectangle {
                    visible: !isEmoji
                    color: highlighted ? border.color : "transparent"
                    border.color: isEmoji ? Kirigami.Theme.focusColor : modelData.color
                    border.width: 2
                    radius: height / 2
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Label {
                        width: Kirigami.Units.gridUnit
                        height: Kirigami.Units.gridUnit
                        visible: isEmoji
                        text: autoCompleteText
                        font.family: "Emoji"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Kirigami.Avatar {
                        Layout.preferredWidth: Kirigami.Units.gridUnit
                        Layout.preferredHeight: Kirigami.Units.gridUnit
                        source: modelData.avatarMediaId ? "image://mxc/" + modelData.avatarMediaId : ""
                        color: modelData.color ? Qt.darker(modelData.color, 1.1) : null
                    }
                    Label {
                        Layout.fillHeight: true

                        visible: !isEmoji
                        text: autoCompleteText
                        color: highlighted ? Kirigami.Theme.highlightTextColor : Kirigami.Theme.textColor
                        verticalAlignment: Text.AlignVCenter
                        rightPadding: Kirigami.Units.largeSpacing
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        autoCompleteListView.currentIndex = index
                        inputField.replaceAutoComplete(autoCompleteText)
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
                property real progress: 0

                Layout.fillWidth: true

                id: inputField

                wrapMode: Text.Wrap
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
                    if (event.modifiers & Qt.ShiftModifier) {
                        insert(cursorPosition, "\n")
                    } else {
                        postMessage(text)
                        text = ""
                        clearReply()
                        closeAll()
                    }
                }

                Keys.onEscapePressed: closeAll()

                Keys.onBacktabPressed: if (isAutoCompleting) autoCompleteListView.decrementCurrentIndex()

                Keys.onTabPressed: {
                    if (isAutoCompleting) {
                        autoCompleteListView.incrementCurrentIndex()
                    } else {
                        autoCompleteBeginPosition = text.substring(0, cursorPosition).lastIndexOf(" ") + 1
                        var autoCompletePrefix = text.substring(0, cursorPosition).split(" ").pop()
                        if (!autoCompletePrefix) return
                        if (autoCompletePrefix.startsWith(":")) {
                            autoCompleteBeginPosition = text.substring(0, cursorPosition).lastIndexOf(" ") + 1
                            autoCompleteModel = emojiModel.filterModel(autoCompletePrefix)
                            if (autoCompleteModel.length === 0) return
                            isAutoCompleting = true
                            autoCompleteEndPosition = cursorPosition
                        } else {
                            autoCompleteModel = currentRoom.getUsers(autoCompletePrefix)
                            if (autoCompleteModel.length === 0) return
                            isAutoCompleting = true
                            autoCompleteEndPosition = cursorPosition
                        }
                    }
                    replaceAutoComplete(autoCompleteListView.currentItem.autoCompleteText)
                }

                onTextChanged: {
                    timeoutTimer.restart()
                    repeatTimer.start()
                    currentRoom.cachedInput = text

                    if (cursorPosition !== autoCompleteBeginPosition && cursorPosition !== autoCompleteEndPosition) {
                        isAutoCompleting = false
                        autoCompleteListView.currentIndex = 0
                    }
                }

                function replaceAutoComplete(word) {
                    remove(autoCompleteBeginPosition, autoCompleteEndPosition)
                    autoCompleteEndPosition = autoCompleteBeginPosition + word.length
                    insert(cursorPosition, word)
                }

                function postMessage(text) {
                    if(!currentRoom) { return }

                    if (hasAttachment) {
                        currentRoom.uploadFile(attachmentPath, text)
                        clearAttachment()
                        return
                    }

                    if (text.trim().length === 0) { return }

                    var PREFIX_ME = '/me '
                    var PREFIX_NOTICE = '/notice '
                    var PREFIX_RAINBOW = '/rainbow '

                    var messageEventType = RoomMessageEvent.Text

                    if (text.indexOf(PREFIX_RAINBOW) === 0) {
                        text = text.substr(PREFIX_RAINBOW.length)

                        var parsedText = ""
                        var rainbowColor = ["#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500", "#ffff00", "#d4ff00", "#aaff00", "#80ff00", "#55ff00", "#2bff00", "#00ff00", "#00ff2b", "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff", "#00d4ff", "#00aaff", "#007fff", "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff", "#aa00ff", "#d400ff", "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"]
                        for (var i = 0; i < text.length; i++) {
                            parsedText = parsedText + "<font color='" + rainbowColor[i % rainbowColor.length] + "'>" + text.charAt(i) + "</font>"
                        }
                        currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text, replyEventID)
                        return
                    }

                    if (text.indexOf(PREFIX_ME) === 0) {
                        text = text.substr(PREFIX_ME.length)
                        messageEventType = RoomMessageEvent.Emote
                    } else if (text.indexOf(PREFIX_NOTICE) === 0) {
                        text = text.substr(PREFIX_NOTICE.length)
                        messageEventType = RoomMessageEvent.Notice
                    }

                    currentRoom.postArbitaryMessage(text, messageEventType, replyEventID)
                }
            }


            ToolButton {
                id: emojiButton
                icon.name: "preferences-desktop-emoticons"
                icon.color: "transparent"

                onClicked: emojiPicker.visible = !emojiPicker.visible
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
