// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.chatbar

/**
 * @brief A component that provides a set of actions when a message is hovered in the timeline.
 *
 * There is also an icon to show that a message has come from a verified device in
 * encrypted chats.
 */
QQC2.Control {
    id: root

    /**
     * @brief The current message delegate the actions are being shown on.
     */
    property var delegate: null

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    /**
     * @brief Whether the actions should be shown.
     */
    readonly property bool showActions: delegate && delegate.hovered

    /**
     * @brief Request that the chat bar be focussed.
     */
    signal focusChatBar

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    visible: (root.hovered || root.showActions || showActionsTimer.running) && !Kirigami.Settings.isMobile && (!root.delegate.isThreaded || !NeoChatConfig.threads)
    onVisibleChanged: {
        if (visible) {
            // HACK: delay disapearing by 200ms, otherwise this can create some glitches
            // See https://invent.kde.org/network/neochat/-/issues/333
            showActionsTimer.restart();
        }
    }
    Timer {
        id: showActionsTimer
        interval: 200
    }

    function updatePosition(): void {
        if (delegate) {
            root.x = delegate.contentItem.x + delegate.bubbleX + delegate.bubbleWidth - root.implicitWidth;
            root.y = delegate.mapToItem(parent, 0, 0).y + delegate.bubbleY - height + Kirigami.Units.smallSpacing;
        }
    }

    onDelegateChanged: updatePosition()
    onWidthChanged: updatePosition()

    contentItem: RowLayout {
        id: actionsLayout

        spacing: Kirigami.Units.smallSpacing

        Item {
            Layout.fillWidth: true
        }

        Kirigami.Icon {
            source: "security-high"
            width: height
            height: root.height
            visible: root.delegate && root.delegate.verified
            HoverHandler {
                id: hover
            }

            QQC2.ToolTip.text: i18n("This message was sent from a verified device")
            QQC2.ToolTip.visible: hover.hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            text: i18n("React")
            icon.name: "preferences-desktop-emoticons"
            onClicked: emojiDialog.open()
            display: QQC2.ToolButton.IconOnly

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            visible: root.delegate && root.delegate.isEditable && !root.currentRoom.readOnly
            text: i18n("Edit")
            icon.name: "document-edit"
            display: QQC2.Button.IconOnly

            onClicked: {
                root.currentRoom.editCache.editId = root.delegate.eventId;
                root.currentRoom.mainCache.replyId = "";
                root.currentRoom.mainCache.threadId = "";
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            visible: !root.currentRoom.readOnly
            text: i18n("Reply")
            icon.name: "mail-replied-symbolic"
            display: QQC2.Button.IconOnly
            onClicked: {
                root.currentRoom.mainCache.replyId = root.delegate.eventId;
                root.currentRoom.editCache.editId = "";
                root.currentRoom.mainCache.threadId = "";
                root.focusChatBar();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            visible: NeoChatConfig.threads && !root.currentRoom.readOnly && !root.delegate?.isPoll
            text: i18n("Reply in Thread")
            icon.name: "dialog-messages"
            display: QQC2.Button.IconOnly
            onClicked: {
                root.currentRoom.threadCache.replyId = "";
                root.currentRoom.threadCache.threadId = root.delegate.isThreaded ? root.delegate.threadRoot : root.delegate.eventId;
                root.currentRoom.mainCache.clearRelations();
                root.currentRoom.editCache.clearRelations();
                root.focusChatBar();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            visible: (root.delegate?.isPoll ?? false) && !ContentProvider.handlerForPoll(root.currentRoom, root.delegate.eventId).hasEnded
            text: i18n("End Poll")
            icon.name: "gtk-stop"
            display: QQC2.ToolButton.IconOnly
            onClicked: root.currentRoom.poll(root.delegate.eventId).endPoll()

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        EmojiDialog {
            id: emojiDialog
            currentRoom: root.currentRoom
            showQuickReaction: true
            showStickers: false
            onChosen: emoji => {
                root.currentRoom.toggleReaction(root.delegate.eventId, emoji);
                if (!Kirigami.Settings.isMobile) {
                    root.focusChatBar();
                }
            }
        }
    }
}
