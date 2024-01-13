// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.neochat

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
    signal focusChatBar()

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    x: delegate ? delegate.contentX + delegate.bubbleX : 0
    y: delegate ? delegate.mapToItem(parent, 0, 0).y + delegate.bubbleY - height + Kirigami.Units.smallSpacing : 0
    width: delegate ? delegate.bubbleWidth : Kirigami.Units.gridUnit * 4

    visible: (root.hovered || root.showActions || showActionsTimer.running) && !Kirigami.Settings.isMobile
    onVisibleChanged: {
        if (visible) {
            // HACK: delay disapearing by 200ms, otherwise this can create some glitches
            // See https://invent.kde.org/network/neochat/-/issues/333
            showActionsTimer.restart()
        }
    }
    Timer {
        id: showActionsTimer
        interval: 200
    }

    contentItem: RowLayout {
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
        Kirigami.ActionToolBar {
            Layout.maximumWidth: maximumContentWidth + Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            alignment: Qt.AlignRight
            flat: false
            display: QQC2.Button.IconOnly

            actions: [
                Kirigami.Action {
                    text: i18n("React")
                    icon.name: "preferences-desktop-emoticons"
                    onTriggered: emojiDialog.open()
                },
                Kirigami.Action {
                    visible: root.delegate && root.delegate.author.isLocalUser && (root.delegate.delegateType === DelegateType.Emote || root.delegate.delegateType === DelegateType.Message) && !root.currentRoom.readOnly
                    text: i18n("Edit")
                    icon.name: "document-edit"
                    onTriggered: {
                        root.currentRoom.editCache.editId = root.delegate.eventId;
                        root.currentRoom.mainCache.replyId = "";
                    }
                },
                Kirigami.Action {
                    visible: !root.currentRoom.readOnly
                    text: i18n("Reply")
                    icon.name: "mail-replied-symbolic"
                    onTriggered: {
                        root.currentRoom.mainCache.replyId = root.delegate.eventId;
                        root.currentRoom.editCache.editId = "";
                        root.focusChatBar();
                    }
                },
                Kirigami.Action {
                    visible: !root.currentRoom.readOnly
                    text: i18n("Reply in Thread")
                    icon.name: "dialog-messages"
                    onTriggered: {
                        root.currentRoom.mainCache.replyId = root.delegate.eventId;
                        root.currentRoom.mainCache.threadId = root.delegate.isThreaded ? root.delegate.threadRoot : root.delegate.eventId;
                        root.currentRoom.editCache.editId = "";
                        root.focusChatBar();
                    }
                }
            ]

            EmojiDialog {
                id: emojiDialog
                currentRoom: root.currentRoom
                showQuickReaction: true
                onChosen: (emoji) => {
                    root.currentRoom.toggleReaction(root.delegate.eventId, emoji);
                    if (!Kirigami.Settings.isMobile) {
                        root.focusChatBar();
                    }
                }
            }
        }
    }
}
