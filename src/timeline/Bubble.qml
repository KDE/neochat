// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A chat bubble for displaying the content of message events.
 *
 * The content of the bubble is set via the content property which is then managed
 * by the bubble to apply the correct sizing (including limiting the width if a
 * maxContentWidth is set).
 *
 * The bubble also supports a header with the author and message timestamp and a
 * reply.
 */
QQC2.Control {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The index of the delegate in the model.
     */
    required property var index

    /**
     * @brief The message author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the author.
     *  - isLocalUser - Whether the author is the local user.
     *  - avatarSource - The mxc URL for the author's avatar in the current room.
     *  - avatarMediaId - The media ID of the author's avatar.
     *  - avatarUrl - The mxc URL for the author's avatar.
     *  - displayName - The display name of the author.
     *  - display - The name of the author.
     *  - color - The color for the author.
     *  - object - The Quotient::User object for the author.
     *
     * @sa Quotient::User
     */
    property var author

    /**
     * @brief Whether the author should be shown.
     */
    required property bool showAuthor

    /**
     * @brief The timestamp of the message.
     */
    property var time

    /**
     * @brief The timestamp of the message as a string.
     */
    property string timeString

    /**
     * @brief Whether the message should be highlighted.
     */
    property bool showHighlight: false

    /**
     * @brief The model to visualise the content of the message.
     */
    required property MessageContentModel contentModel

    /**
     * @brief The ActionsHandler object to use.
     *
     * This is expected to have the correct room set otherwise messages will be sent
     * to the wrong room.
     */
    property ActionsHandler actionsHandler

    /**
     * @brief Whether the bubble background should be shown.
     */
    property alias showBackground: bubbleBackground.visible

    /**
     * @brief The timeline ListView this component is being used in.
     */
    required property ListView timeline

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief Request a context menu be show for the message.
     */
    signal showMessageMenu

    contentItem: ColumnLayout {
        id: contentColumn
        spacing: Kirigami.Units.smallSpacing
        RowLayout {
            id: headerRow
            Layout.maximumWidth: root.maxContentWidth
            implicitHeight: Math.max(nameButton.implicitHeight, timeLabel.implicitHeight)
            visible: root.showAuthor
            QQC2.AbstractButton {
                id: nameButton
                Layout.fillWidth: true
                contentItem: QQC2.Label {
                    text: root.author.displayName
                    color: root.author.color
                    textFormat: Text.PlainText
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }
                Accessible.name: contentItem.text
                onClicked: RoomManager.resolveResource(root.author.id, "mention")
            }
            QQC2.Label {
                id: timeLabel
                text: root.timeString
                horizontalAlignment: Text.AlignRight
                color: Kirigami.Theme.disabledTextColor
                QQC2.ToolTip.visible: timeHoverHandler.hovered
                QQC2.ToolTip.text: root.time.toLocaleString(Qt.locale(), Locale.LongFormat)
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                HoverHandler {
                    id: timeHoverHandler
                }
            }
        }
        Repeater {
            id: contentRepeater
            model: root.contentModel
            delegate: MessageComponentChooser {
                room: root.room
                index: root.index
                actionsHandler: root.actionsHandler
                timeline: root.timeline
                maxContentWidth: root.maxContentWidth

                onReplyClicked: eventId => {
                    root.replyClicked(eventId);
                }
                onSelectedTextChanged: selectedText => {
                    root.selectedTextChanged(selectedText);
                }
                onShowMessageMenu: root.showMessageMenu()
            }
        }
    }

    background: Kirigami.ShadowedRectangle {
        id: bubbleBackground
        visible: root.showBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: if (root.author.isLocalUser) {
            return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15);
        } else if (root.showHighlight) {
            return Kirigami.Theme.positiveBackgroundColor;
        } else {
            return Kirigami.Theme.backgroundColor;
        }
        radius: Kirigami.Units.smallSpacing
        shadow {
            size: Kirigami.Units.smallSpacing
            color: root.showHighlight ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
        }

        Behavior on color {
            ColorAnimation {
                duration: Kirigami.Units.shortDuration
            }
        }
    }
}
