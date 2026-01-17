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

    onActiveFocusChanged: chatContentView.itemAt(contentModel.index(contentModel.focusRow, 0)).forceActiveFocus()

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

    Message.room: root.currentRoom
    Message.contentModel: contentModel

    implicitHeight: chatBar.implicitHeight + Kirigami.Units.largeSpacing

    QQC2.Control {
        id: chatBar

        anchors.top: root.top
        anchors.horizontalCenter: root.horizontalCenter

        spacing: 0

        width: chatBarSizeHelper.availableWidth - Kirigami.Units.largeSpacing * 2
        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing

        contentItem: ColumnLayout {
            RichEditBar {
                id: richEditBar
                visible: NeoChatConfig.sendMessageWith === 1
                maxAvailableWidth: chatBarSizeHelper.availableWidth - Kirigami.Units.largeSpacing * 2

                room: root.currentRoom
                contentModel: chatContentView.model

                onClicked: contentModel.refocusCurrentComponent()
            }
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: NeoChatConfig.sendMessageWith === 1
            }
            RowLayout {
                spacing: 0
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
                                sendMessageWithEnter: NeoChatConfig.sendMessageWith === 0
                            }

                            delegate: MessageComponentChooser {}
                        }
                    }
                }
                SendBar {
                    room: root.currentRoom
                    contentModel: chatContentView.model
                }
            }
        }

        background: Kirigami.ShadowedRectangle {
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor
            border {
                color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
                width: 1
            }
        }
    }
    MouseArea {
        id: hoverArea
        anchors {
            top: chatModeButton.top
            left: root.left
            right: root.right
            bottom: chatBar.top
        }
        propagateComposedEvents: true
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }
    QQC2.Button {
        id: chatModeButton
        anchors {
            bottom: chatBar.top
            bottomMargin: Kirigami.Units.smallSpacing
            horizontalCenter: root.horizontalCenter
        }

        visible: hoverArea.containsMouse || hovered || chatBar.hovered
        width: Kirigami.Units.iconSizes.enormous
        height: Kirigami.Units.iconSizes.smallMedium

        icon.name: NeoChatConfig.sendMessageWith === 0 ? "arrow-up" : "arrow-down"

        onClicked: NeoChatConfig.sendMessageWith = NeoChatConfig.sendMessageWith === 0 ? 1 : 0
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

        property LibNeoChat.CompletionModel completionModel: LibNeoChat.CompletionModel {
            room: root.currentRoom
            type: LibNeoChat.ChatBarType.Room
            textItem: contentModel.focusedTextItem
            roomListModel: RoomManager.roomListModel
            userListModel: RoomManager.userListModel

            onIsCompletingChanged: {
                if (!isCompleting) {
                    return;
                }

                let dialog = Qt.createComponent('org.kde.neochat.chatbar', 'CompletionMenu').createObject(contentModel.focusedTextItem.textItem, {
                    model: _private.completionModel,
                    keyHelper: contentModel.keyHelper
                }).open();
            }
        }
    }
}
