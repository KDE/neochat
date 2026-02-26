// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

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
Item {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property LibNeoChat.NeoChatRoom currentRoom
    onCurrentRoomChanged: {
        if (ShareHandler.text.length > 0 && ShareHandler.room === root.currentRoom.id) {
            contentModel.focusedTextItem.
            textField.text = ShareHandler.text;
            ShareHandler.text = "";
            ShareHandler.room = "";
        }
    }

    onActiveFocusChanged: if (activeFocus) {
        core.forceActiveFocus();
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

        function onMentionAdded(text: string, hRef: string): void {
            core.completionModel.insertCompletion(text, hRef);
            // move the focus back to the chat bar
            core.model.refocusCurrentComponent();
        }
    }

    implicitHeight: column.implicitHeight + Kirigami.Units.largeSpacing

    ColumnLayout {
        id: column
        anchors.top: root.top
        anchors.horizontalCenter: root.horizontalCenter
        ChatBarCore {
            id: core
            Message.room: root.currentRoom
            room: root.currentRoom
            maxAvailableWidth: chatBarSizeHelper.availableWidth
        }
        QQC2.Label {
            Layout.fillWidth: true
            visible: !Kirigami.Setting.isMobile
            text: NeoChatConfig.sendMessageWith === 1 ? i18nc("As in enter starts a new line in the chat bar", "Enter starts a new line") : i18nc("As in enter starts send the chat message", "Enter sends the message")
            horizontalAlignment: Text.AlignRight
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * NeoChatConfig.fontScale * 0.75
        }
    }
    LibNeoChat.DelegateSizeHelper {
        id: chatBarSizeHelper
        parentItem: root
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: NeoChatConfig.compactLayout ? 100 : 85
        leftPadding: NeoChatConfig.compactLayout ? Kirigami.Units.largeSpacing * 2 : 0 
        rightPadding: NeoChatConfig.compactLayout ? Kirigami.Units.largeSpacing * 2 : 0 
        maxWidth: NeoChatConfig.compactLayout ? root.width - Kirigami.Units.largeSpacing * 2 : Kirigami.Units.gridUnit * 60
    }
}
