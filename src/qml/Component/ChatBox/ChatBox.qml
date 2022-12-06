// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

ColumnLayout {
    id: chatBox

    property alias inputFieldText: chatBar.inputFieldText

    signal messageSent()

    spacing: 0

    Kirigami.Separator {
        id: connectionPaneSeparator
        visible: connectionPane.visible
        Layout.fillWidth: true
    }

    QQC2.Pane {
        id: connectionPane
        padding: fontMetrics.lineSpacing * 0.25
        FontMetrics {
            id: fontMetrics
            font: networkLabel.font
        }
        spacing: 0
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }
        visible: !Controller.isOnline
        Layout.fillWidth: true
        QQC2.Label {
            id: networkLabel
            width: parent.width
            wrapMode: Text.Wrap
            text: i18n("NeoChat is offline. Please check your network connection.")
        }
    }

    Kirigami.Separator {
        id: replySeparator
        visible: replyPane.visible
        Layout.fillWidth: true
    }

    ReplyPane {
        id: replyPane
        visible: currentRoom.chatBoxReplyId.length > 0 || currentRoom.chatBoxEditId.length > 0
        Layout.fillWidth: true

        onReplyCancelled: {
            chatBox.focusInputField()
        }
    }

    Kirigami.Separator {
        id: attachmentSeparator
        visible: attachmentPane.visible
        Layout.fillWidth: true
    }

    AttachmentPane {
        id: attachmentPane
        visible: currentRoom.chatBoxAttachmentPath.length > 0
        Layout.fillWidth: true
    }

    Kirigami.Separator {
        id: chatBarSeparator
        visible: chatBar.visible

        Layout.fillWidth: true
    }

    ChatBar {
        id: chatBar
        visible: currentRoom.canSendEvent("m.room.message")

        Layout.fillWidth: true

        onCloseAllTriggered: closeAll()
        onMessageSent: {
            closeAll()
            chatBox.messageSent();
        }

        Behavior on implicitHeight {
            NumberAnimation {
                property: "implicitHeight"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    function insertText(str) {
        let index = chatBar.cursorPosition;
        chatBox.inputFieldText = inputFieldText.substr(0, chatBar.cursorPosition) + str + inputFieldText.substr(chatBar.cursorPosition);
        chatBar.cursorPosition = index + str.length;
    }

    function focusInputField() {
        chatBar.inputFieldForceActiveFocusTriggered()
    }

    function closeAll() {
        // TODO clear();
        chatBar.emojiPaneOpened = false;
    }
}
