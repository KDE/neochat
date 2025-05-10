// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

RowLayout {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    property real availableWidth: 0.0

    property bool reacting: false

    QQC2.Button {
        id: reactButton
        visible: root.availableWidth > overflowButton.implicitWidth + root.spacing + reactButton.implicitWidth
        text: i18n("React")
        icon.name: "preferences-desktop-emoticons"
        display: QQC2.ToolButton.IconOnly
        onClicked: {
            var dialog = emojiDialog.createObject(reactButton);
            dialog.chosen.connect(emoji => {
                root.reacting = false;
                root.room.toggleReaction(root.eventId, emoji);
                if (!Kirigami.Settings.isMobile) {
                    // root.focusChatBar();
                }
            });
            dialog.closed.connect(() => {
                root.reacting = false;
            })
            root.reacting = true;
            dialog.open();
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        Component {
            id: emojiDialog
            EmojiDialog {
                currentRoom: root.room
                showQuickReaction: true
            }
        }
    }
    QQC2.Button {
        id: replyButton
        visible: !root.room.readOnly && root.availableWidth > overflowButton.implicitWidth + reactButton.implicitWidth + replyButton.implicitWidth + root.spacing
        text: i18n("Reply")
        icon.name: "mail-replied-symbolic"
        display: QQC2.Button.IconOnly
        onClicked: {
            root.room.mainCache.replyId = root.eventId;
            root.room.editCache.editId = "";
            root.room.mainCache.threadId = "";
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    QQC2.Button {
        id: overflowButton
        text: i18n("Message menu")
        icon.name: "overflow-menu"
        onClicked: _private.showMessageMenu()
        display: QQC2.ToolButton.IconOnly

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
