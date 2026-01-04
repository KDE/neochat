// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
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
QQC2.Control {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    required property NeoChatConnection connection

    onActiveFocusChanged: chatContentView.itemAt(contentModel.index(contentModel.focusRow, 0)).forceActiveFocus()

    onCurrentRoomChanged: {
        _private.chatBarCache = currentRoom.mainCache
        if (ShareHandler.text.length > 0 && ShareHandler.room === root.currentRoom.id) {
            contentModel.focusedTextItem.
            textField.text = ShareHandler.text;
            ShareHandler.text = "";
            ShareHandler.room = "";
        }
    }

    Connections {
        target: contentModel.keyHelper

        function onUnhandledUp(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            completionMenu.decrementIndex();
        }

        function onUnhandledDown(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            completionMenu.incrementIndex();
        }

        function onUnhandledTab(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            completionMenu.completeCurrent();
        }

        function onUnhandledReturn(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            completionMenu.completeCurrent();
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

    Connections {
        target: root.currentRoom.mainCache

        function onMentionAdded(text: string, hRef: string): void {
            completionMenu.complete(text, hRef);
            // move the focus back to the chat bar
            contentModel.refocusCurrentComponent();
        }
    }

    spacing: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Message.room: root.currentRoom
    Message.contentModel: contentModel

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Separator {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
        }
    }

    height: Math.max(Math.min(chatScrollView.contentHeight + bottomPadding + topPadding, Kirigami.Units.gridUnit * 10), Kirigami.Units.gridUnit * 5)
    leftPadding: rightPadding
    rightPadding: (root.width - chatBarSizeHelper.availableWidth) / 2
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    contentItem: ColumnLayout {
        QQC2.ScrollView {
            id: chatScrollView
            Layout.fillWidth: true
            Layout.maximumHeight: Kirigami.Units.gridUnit * 8

            clip: true

            ColumnLayout {
                width: chatScrollView.width
                spacing: Kirigami.Units.smallSpacing

                Repeater {
                    id: chatContentView
                    model: ChatBarMessageContentModel {
                        id: contentModel
                        type: ChatBarType.Room
                        room: root.currentRoom
                    }

                    delegate: MessageComponentChooser {}
                }
            }
        }
        RichEditBar {
            id: richEditBar
            Layout.alignment: Qt.AlignCenter
            maxAvailableWidth: chatBarSizeHelper.availableWidth - Kirigami.Units.largeSpacing * 2

            room: root.currentRoom
            contentModel: chatContentView.model

            onClicked: contentModel.refocusCurrentComponent()
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

    QtObject {
        id: _private
        property ChatBarCache chatBarCache
        onChatBarCacheChanged: {
            richEditBar.chatBarCache = chatBarCache
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

    CompletionMenu {
        id: completionMenu
        room: root.currentRoom
        type: LibNeoChat.ChatBarType.Room
        textItem: contentModel.focusedTextItem

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
}
