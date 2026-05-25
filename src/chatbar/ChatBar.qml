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
            core.model.focusedTextItem.textItem.text = ShareHandler.text;
            ShareHandler.text = "";
            ShareHandler.room = "";
        }
    }

    property alias model: core.model

    onActiveFocusChanged: if (activeFocus) {
        core.forceActiveFocus();
    }

    Connections {
        target: ShareHandler
        function onRoomChanged(): void {
            if (ShareHandler.text.length > 0 && ShareHandler.room === root.currentRoom.id) {
                core.model.focusedTextItem.textItem.text = ShareHandler.text;
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

    implicitHeight: column.implicitHeight + Kirigami.Units.largeSpacing + parent.SafeArea.margins.bottom

    ColumnLayout {
        id: column

        anchors.top: root.top
        anchors.horizontalCenter: root.horizontalCenter

        ChatBarCore {
            id: core
            Message.room: root.currentRoom
            room: root.currentRoom
            maxAvailableWidth: chatBarSizeHelper.availableWidth
            visible: !root.currentRoom.readOnly
        }
        QQC2.Label {
            visible: root.currentRoom.readOnly
            text: i18nc("@info:label", "You do not have permission to send messages to this room.")
            color: Kirigami.Theme.disabledTextColor
            elide: Text.ElideRight

            Layout.fillWidth: true
            Layout.preferredWidth: chatBarSizeHelper.availableWidth
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: chatBarSizeHelper.availableWidth // Ensure our labels below are elided properly

            visible: !Kirigami.Settings.isMobile

            FontMetrics {
                id: fontMetrics
            }

            TypingPane {
                id: typingPane
                room: root.currentRoom
                drawBackground: false
                visible: root.currentRoom && root.currentRoom.otherMembersTyping.length > 0

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                Layout.preferredHeight: fontMetrics.height // Prevent layout shifts with the typing indicator loading/unloading
            }
            QQC2.Label {
                text: NeoChatConfig.sendMessageWith === 1 ? i18nc("As in enter starts a new line in the chat bar", "Enter starts a new line") : i18nc("As in enter starts send the chat message", "Enter sends the message")
                horizontalAlignment: Text.AlignRight
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * NeoChatConfig.fontScale * 0.75
                visible: !root.currentRoom.readOnly

                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: !typingPane.visible
                Layout.preferredHeight: fontMetrics.height // Prevent layout shifts with the typing indicator loading/unloading
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
        leftPadding: 0
        rightPadding: 0
        maxWidth: NeoChatConfig.compactLayout ? root.width - Kirigami.Units.smallSpacing * 2 : Kirigami.Units.gridUnit * 60
    }
}
