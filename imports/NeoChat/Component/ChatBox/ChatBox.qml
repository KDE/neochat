/* SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import Qt.labs.platform 1.0 as Platform
import org.kde.kirigami 2.14 as Kirigami

import NeoChat.Component.ChatBox 1.0
import NeoChat.Component.Emoji 1.0
import org.kde.neochat 1.0

Item {
    id: root
    readonly property bool isReply: replyEventId.length > 0
    property var replyUser
    property alias replyEventId: chatBar.replyEventId
    property string replyContent: ""

    readonly property bool hasAttachment: attachmentPath.length > 0
    property string attachmentPath: ""

    property alias inputFieldText: chatBar.inputFieldText

    readonly property bool isEdit: editEventId.length > 0
    property alias editEventId: chatBar.editEventId

    signal fancyEffectsReasonFound(string fancyEffect)

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    implicitWidth: {
        let w = 0
        for(let i = 0; i < visibleChildren.length; ++i) {
            w = Math.max(w, Math.ceil(visibleChildren[i].implicitWidth))
        }
        return w
    }
    implicitHeight: {
        let h = 0
        for(let i = 0; i < visibleChildren.length; ++i) {
            h += Math.ceil(visibleChildren[i].implicitHeight)
        }
        return h
    }

    // For some reason, this is needed to make the height animation work even though
    // it used to work and height should be directly affected by implicitHeight
    height: implicitHeight

    Behavior on height {
        NumberAnimation {
            property: "height"
            duration: Kirigami.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }

    Kirigami.Separator {
        id: emojiPickerLoaderSeparator
        visible: emojiPickerLoader.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: emojiPickerLoader.top
        z: 1
    }

    Loader {
        id: emojiPickerLoader
        active: visible
        visible: chatBar.emojiPaneOpened
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: replySeparator.top
        sourceComponent: EmojiPicker{
            textArea: chatBar.textField
            emojiModel: EmojiModel { id: emojiModel }
        }
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }

    }

    Kirigami.Separator {
        id: replySeparator
        visible: replyPane.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: replyPane.top
    }

    ReplyPane {
        id: replyPane
        visible: isReply || isEdit
        isEdit: root.isEdit
        user: root.replyUser
        content: root.replyContent
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: attachmentSeparator.top
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    Kirigami.Separator {
        id: attachmentSeparator
        visible: attachmentPane.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: attachmentPane.top
    }

    AttachmentPane {
        id: attachmentPane
        attachmentPath: root.attachmentPath
        visible: hasAttachment
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: chatBarSeparator.top
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    Kirigami.Separator {
        id: chatBarSeparator
        visible: chatBar.visible
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: chatBar.top
    }

    ChatBar {
        id: chatBar
        visible: currentRoom.canSendEvent("m.room.message")
        width: parent.width
        height: visible ? implicitHeight : 0
        anchors.bottom: parent.bottom

        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    Connections {
        target: replyPane
        function onClearEditReplyTriggered() {
            if (isEdit) {
                clearEdit()
            }
            if (isReply) {
                clearReply()
            }
        }
    }

    Connections {
        target: attachmentPane
        function onClearAttachmentTriggered() {
            clearAttachment()
        }
    }

    Connections {
        target: chatBar
        function onAttachTriggered(localPath) {
            attach(localPath)
        }
        function onCloseAllTriggered() {
            closeAll()
        }
        function onMessageSent() {
            closeAll()
            checkForFancyEffectsReason()
        }
    }

    function checkForFancyEffectsReason() {
        if (!Config.showFancyEffects) {
            return
        }

        let text = root.inputFieldText.trim()
        if (text.includes('\u{2744}')) {
            root.fancyEffectsReasonFound("snowflake")
        }
        if (text.includes('\u{1F386}')) {
            root.fancyEffectsReasonFound("fireworks")
        }
        if (text.includes('\u{1F387}')) {
            root.fancyEffectsReasonFound("fireworks")
        }
        if (text.includes('\u{1F389}')) {
            root.fancyEffectsReasonFound("confetti")
        }
        if (text.includes('\u{1F38A}')) {
            root.fancyEffectsReasonFound("confetti")
        }
    }

    function addText(text) {
        root.inputFieldText = inputFieldText + text
    }

    function insertText(str) {
        root.inputFieldText = inputFieldText.substr(0, inputField.cursorPosition) + str + inputFieldText.substr(inputField.cursorPosition)
    }

    function clearText() {
        // ChatBar's TextArea syncs currentRoom.cachedInput with the TextArea's text property
        root.inputFieldText = ""
    }

    function focusInputField() {
        chatBar.inputFieldForceActiveFocusTriggered()
    }

    function edit(editContent, editFormatedContent, editEventId) {
        // Set the input field in edit mode
        root.inputFieldText = editContent;
        root.editEventId = editEventId;
        root.replyContent = editContent;

        // clean autocompletion list
        chatBar.userAutocompleted = {};

        // Fill autocompletion list with values extracted from message.
        // We can't just iterate on every user in the list and try to
        // find matching display name since some users have display name
        // matching frequent words and this will marks too many words as
        // mentions.
        const regex = /<a href=\"https:\/\/matrix.to\/#\/(@[a-zA-Z09]*:[a-zA-Z09.]*)\">([^<]*)<\/a>/g;

        let match;
        while ((match = regex.exec(editFormatedContent.toString())) !== null) {
            chatBar.userAutocompleted[match[2]] = match[1];
        }
    }

    function clearEdit() {
        // Clear input when edits are cancelled.
        // Cached input will be
        clearText()
        clearReply()
        root.editEventId = "";
    }

    function attach(localPath) {
        root.attachmentPath = localPath
    }

    function clearAttachment() {
        root.attachmentPath = ""
    }

    function clearReply() {
        replyUser = null;
        root.replyContent = "";
        root.replyEventId = "";
        // Don't clear input when replies are cancelled
    }

    function closeAll() {
        if (hasAttachment) {
            clearAttachment();
        }
        if (isEdit) {
            clearEdit();
        }
        if (isReply) {
            clearReply();
        }
        chatBar.emojiPaneOpened = false;
    }
}
