// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.libneochat as LibNeoChat

QQC2.Control {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property LibNeoChat.NeoChatRoom room

    property int chatBarType: LibNeoChat.ChatBarType.Room

    required property real maxAvailableWidth

    readonly property ChatBarMessageContentModel model: ChatBarMessageContentModel {
        type: root.chatBarType
        room: root.room
        sendMessageWithEnter: NeoChatConfig.sendMessageWith === 0
        sendTypingNotifications: NeoChatConfig.typingNotifications
    }

    readonly property LibNeoChat.CompletionModel completionModel: LibNeoChat.CompletionModel {
        textItem: root.model.focusedTextItem
        roomListModel: RoomManager.roomListModel
        userListModel: RoomManager.userListModel

        onIsCompletingChanged: {
            if (!isCompleting) {
                return;
            }

            let dialog = Qt.createComponent('org.kde.neochat.chatbar', 'CompletionMenu').createObject(root.model.focusedTextItem.textItem, {
                model: root.completionModel,
                keyHelper: root.model.keyHelper
            }).open();
        }
    }

    Message.contentModel: root.model

    onActiveFocusChanged: root.model.refocusCurrentComponent()

    implicitWidth: root.maxAvailableWidth - (root.maxAvailableWidth >= (parent?.width ?? 0) ? Kirigami.Units.largeSpacing * 2 : 0)
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    contentItem: ColumnLayout {
        RichEditBar {
            id: richEditBar
            visible: NeoChatConfig.sendMessageWith === 1
            maxAvailableWidth: root.maxAvailableWidth - Kirigami.Units.largeSpacing * 2

            room: root.room
            contentModel: root.model

            onClicked: root.model.refocusCurrentComponent()
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: NeoChatConfig.sendMessageWith === 1
        }
        RowLayout {
            QQC2.ToolButton {
                id: emojiButton
                property EmojiDialog dialog
                
                icon.name: "smiley"
                text: i18n("Emojis & Stickers")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: dialog !== null
        
                onClicked: {
                    if(!checked){
                        if(dialog) {
                            dialog.close();
                        }
                        return;
                    }
                    
                    dialog = (emojiDialog.createObject(root) as EmojiDialog);
                    dialog.onClosed.connect(() => {
                        dialog = null;
                    });
                    dialog.open();
                }
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.text: text
            }
            
            QQC2.ScrollView {
                id: chatScrollView
                Layout.fillWidth: true
                Layout.maximumHeight: Kirigami.Units.gridUnit * (root.model.hasAttachment ? 12 : 8)

                contentWidth: availableWidth
                clip: true

                ColumnLayout {
                    readonly property real visibleTop: chatScrollView.QQC2.ScrollBar.vertical.position * chatScrollView.contentHeight
                    readonly property real visibleBottom: chatScrollView.QQC2.ScrollBar.vertical.position * chatScrollView.contentHeight + chatScrollView.QQC2.ScrollBar.vertical.size * chatScrollView.contentHeight
                    readonly property rect cursorRectInColumn: mapFromItem(root.model.focusedTextItem.textItem, root.model.focusedTextItem.cursorRectangle);
                    onCursorRectInColumnChanged: {
                        if (chatScrollView.QQC2.ScrollBar.vertical.visible) {
                            if (cursorRectInColumn.y < visibleTop) {
                                chatScrollView.QQC2.ScrollBar.vertical.position = cursorRectInColumn.y / chatScrollView.contentHeight
                            } else if (cursorRectInColumn.y + cursorRectInColumn.height > visibleBottom) {
                                chatScrollView.QQC2.ScrollBar.vertical.position = (cursorRectInColumn.y + cursorRectInColumn.height - (chatScrollView.QQC2.ScrollBar.vertical.size * chatScrollView.contentHeight)) / chatScrollView.contentHeight
                            }
                        }
                    }

                    width: chatScrollView.width
                    spacing: Kirigami.Units.smallSpacing

                    Repeater {
                        id: chatContentView
                        model: root.model

                        delegate: BaseMessageComponentChooser {
                            rightAnchorMargin: chatScrollView.QQC2.ScrollBar.vertical.visible ? chatScrollView.QQC2.ScrollBar.vertical.width : 0
                        }
                    }
                }
            }
            SendBar {
                room: root.room
                contentModel: root.model
                maxAvailableWidth: root.maxAvailableWidth
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
    
    Component {
        id: emojiDialog
        EmojiDialog {
            x: 0
            y: -implicitHeight

            modal: false
            includeCustom: true
            closeOnChosen: false

            currentRoom: root.room

            onChosen: emoji => {
                root.chatButtonHelper.insertText(emoji);
                close();
            }

        }
    }   
}
