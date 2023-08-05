// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

/**
 * @brief A component that provides a set of actions when a message is hovered in the timeline.
 *
 * There is also an icon to show that a message has come from a verified device in
 * encrypted chats.
 */
QQC2.Control {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    /**
     * @brief Whether the actions should be shown.
     */
    property bool showActions: false

    /**
     * @brief Whether the message has been sent from a verified matrix session.
     */
    property bool verified: false

    /**
     * @brief Whether the edit button should be shown.
     */
    property bool editable: false

    /**
     * @brief The react button has been clicked.
     */
    signal reactClicked(string emoji)

    /**
     * @brief The edit button has been clicked.
     */
    signal editClicked()

    /**
     * @brief The reply button has been clicked.
     */
    signal replyClicked()

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

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
            visible: root.verified
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
                    text: i18n("Edit")
                    icon.name: "document-edit"
                    onTriggered: root.editClicked()
                    visible: root.editable
                },
                Kirigami.Action {
                    text: i18n("Reply")
                    icon.name: "mail-replied-symbolic"
                    onTriggered: root.replyClicked()
                }
            ]

            EmojiDialog {
                id: emojiDialog
                currentRoom: root.currentRoom
                showQuickReaction: true
                onChosen: (emoji) => root.reactClicked(emoji)
            }
        }
    }
}
