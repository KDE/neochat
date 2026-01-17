// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Flow {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The reaction model to get the reactions from.
     */
    required property ReactionModel reactionModel

    // HACK: We do not set Layout properties here, see BUG 504344 for the crash it caused.

    spacing: Kirigami.Units.smallSpacing

    Repeater {
        id: reactionRepeater

        model: root.reactionModel

        delegate: QQC2.AbstractButton {
            id: reactionDelegate

            required property string textContent
            required property string reaction
            required property string toolTip
            required property bool hasLocalMember

            width: Math.max(contentItem.implicitWidth + leftPadding + rightPadding, height)
            height: Math.round(Kirigami.Units.gridUnit * 1.5)

            contentItem: QQC2.Label {
                id: reactionLabel
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: reactionDelegate.textContent
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * NeoChatConfig.fontScale
                background: null
                wrapMode: TextEdit.NoWrap
                textFormat: Text.RichText
            }

            padding: Kirigami.Units.smallSpacing

            background: Kirigami.ShadowedRectangle {
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: NeoChatConfig.compactLayout ? Kirigami.Theme.View : Kirigami.Theme.Window
                color: reactionDelegate.hasLocalMember ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
                radius: height / 2
                shadow {
                    size: Kirigami.Units.smallSpacing
                    color: !reactionDelegate.hasLocalMember ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                }
            }

            onClicked: {
                root.Message.room.toggleReaction(root.eventId, reactionDelegate.reaction)
            }

            hoverEnabled: true

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: reactionDelegate.toolTip
        }
    }

    QQC2.AbstractButton {
        id: reactButton
        width: Math.round(Kirigami.Units.gridUnit * 1.5)
        height: Math.round(Kirigami.Units.gridUnit * 1.5)

        text: i18nc("@button", "React")

        contentItem: Kirigami.Icon {
            source: "list-add"
        }

        padding: Kirigami.Units.smallSpacing

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            radius: height / 2
            border {
                width: reactButton.hovered ? 1 : 0
                color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
            }
        }

        onClicked: {
            var dialog = emojiDialog.createObject(reactButton);
            dialog.showStickers = false;
            dialog.chosen.connect(emoji => {
                root.Message.room.toggleReaction(root.eventId, emoji);
                if (!Kirigami.Settings.isMobile) {
                    root.focusChatBar();
                }
            });
            dialog.open();
        }

        hoverEnabled: true

        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: reactButton.text
    }

    Component {
        id: emojiDialog

        EmojiDialog {
            currentRoom: root.Message.room
            showQuickReaction: true
        }
    }
}
