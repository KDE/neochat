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
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    property var author

    /**
     * @brief Whether the message author should be shown.
     */
    required property bool showAuthor

    /**
     * @brief Whether the message should be highlighted.
     */
    property bool showHighlight: false

    /**
     * @brief The model to visualise the content of the message.
     */
    required property var contentModel

    /**
     * @brief Whether the message in a thread.
     */
    required property bool isThreaded

    /**
     * @brief Whether the bubble background should be shown.
     */
    property alias showBackground: bubbleBackground.visible

    required property bool isPending

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief The user hovered link has changed.
     */
    signal hoveredLinkChanged(string hoveredLink)

    /**
     * @brief Request a context menu be show for the message.
     */
    signal showMessageMenu

    Message.contentFilterModel: messageContentFilterModel

    contentItem: RowLayout {
        Kirigami.Icon {
            source: "content-loading-symbolic"
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            visible: root.isPending && NeoChatConfig.showLocalMessagesOnRight
        }
        ColumnLayout {
            id: contentColumn
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                id: contentRepeater
                model: MessageContentFilterModel {
                    id: messageContentFilterModel
                    showAuthor: root.showAuthor
                    sourceModel: root.contentModel
                }
                delegate: MessageComponentChooser {
                    onSelectedTextChanged: selectedText => {
                        root.selectedTextChanged(selectedText);
                    }
                    onHoveredLinkChanged: hoveredLink => {
                        root.hoveredLinkChanged(hoveredLink);
                    }
                    onRemoveLinkPreview: index => root.contentModel.closeLinkPreview(index)
                }
            }
        }
        Kirigami.Icon {
            source: "content-loading-symbolic"
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            visible: root.isPending && !NeoChatConfig.showLocalMessagesOnRight
        }
    }

    background: Kirigami.ShadowedRectangle {
        id: bubbleBackground
        visible: root.showBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: if (root.author.isLocalMember && !root.isThreaded) {
            return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15);
        } else if (root.showHighlight) {
            return Kirigami.Theme.positiveBackgroundColor;
        } else {
            return Kirigami.Theme.backgroundColor;
        }
        radius: Kirigami.Units.cornerRadius
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
