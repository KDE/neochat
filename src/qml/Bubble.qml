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
     * @brief The main delegate content item to show in the bubble.
     */
    property Item content

    /**
     * @brief Whether this message is replying to another.
     */
    property bool isReply: false

    /**
     * @brief The matrix ID of the reply event.
     */
    required property var replyId

    /**
     * @brief The reply author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the reply author.
     *  - isLocalUser - Whether the reply author is the local user.
     *  - avatarSource - The mxc URL for the reply author's avatar in the current room.
     *  - avatarMediaId - The media ID of the reply author's avatar.
     *  - avatarUrl - The mxc URL for the reply author's avatar.
     *  - displayName - The display name of the reply author.
     *  - display - The name of the reply author.
     *  - color - The color for the reply author.
     *  - object - The Quotient::User object for the reply author.
     *
     * @sa Quotient::User
     */
    required property var replyAuthor

    /**
     * @brief The delegate type of the message replied to.
     */
    required property int replyDelegateType

    /**
     * @brief The display text of the message replied to.
     */
    required property string replyDisplay

    /**
     * @brief The media info for the reply event.
     *
     * This could be an image, audio, video or file.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media.
     *  - mimeIcon - The MIME icon name.
     *  - size - The file size in bytes.
     *  - duration - The length in seconds of the audio media (audio/video only).
     *  - width - The width in pixels of the audio media (image/video only).
     *  - height - The height in pixels of the audio media (image/video only).
     *  - tempInfo - mediaInfo (with the same properties as this except no tempInfo) for a temporary image while the file downloads (image/video only).
     */
    required property var replyMediaInfo

    /**
     * @brief Whether the bubble background should be shown.
     */
    property alias showBackground: bubbleBackground.visible

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)

    contentItem: ColumnLayout {
        RowLayout {
            Layout.maximumWidth: root.maxContentWidth
            visible: root.showAuthor
            QQC2.Label {
                Layout.fillWidth: true
                text: root.author.displayName
                color: root.author.color
                textFormat: Text.PlainText
                font.weight: Font.Bold
                elide: Text.ElideRight

                TapHandler {
                    onTapped: RoomManager.visitUser(root.author.object, "mention")
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
            QQC2.Label {
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
        Loader {
            id: replyLoader
            Layout.fillWidth: true
            Layout.maximumWidth: root.maxContentWidth

            active: root.isReply && root.replyDelegateType !== DelegateType.Other
            visible: active

            sourceComponent: ReplyComponent {
                author: root.replyAuthor
                type: root.replyDelegateType
                display: root.replyDisplay
                mediaInfo: root.replyMediaInfo
                contentMaxWidth: root.maxContentWidth
            }

            Connections {
                target: replyLoader.item
                function onReplyClicked() {
                    replyClicked(root.replyId)
                }
            }
        }
        Item {
            id: contentParent
            Layout.fillWidth: true
            Layout.maximumWidth: root.maxContentWidth
            implicitWidth: root.content ? root.content.implicitWidth : 0
            implicitHeight: root.content ? root.content.implicitHeight : 0
        }
    }

    background: Kirigami.ShadowedRectangle {
        id: bubbleBackground
        visible: root.showBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: if (root.author.isLocalUser) {
            return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
        } else if (root.showHighlight) {
            return Kirigami.Theme.positiveBackgroundColor
        } else {
            return Kirigami.Theme.backgroundColor
        }
        radius: Kirigami.Units.smallSpacing
        shadow {
            size:  Kirigami.Units.smallSpacing
            color: root.showHighlight ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
        }

        Behavior on color {
            ColorAnimation { duration: Kirigami.Units.shortDuration }
        }
    }

    onContentChanged: {
        if (!root.content) {
            return;
        }
        root.content.parent = contentParent;
        root.content.anchors.fill = contentParent;
    }
}
