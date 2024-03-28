// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.coreaddons
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

/**
 * @brief A component to show a message that has been replied to.
 *
 * Similar to the main timeline delegate a reply delegate is chosen based on the type
 * of message being replied to. The main difference is that not all messages can be
 * show in their original form and are instead visualised with a MIME type delegate
 * e.g. Videos.
 */
RowLayout {
    id: root

    /**
     * @brief The matrix ID of the reply event.
     */
    required property var replyComponentType

    /**
     * @brief The matrix ID of the reply event.
     */
    required property var replyEventId

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

    implicitHeight: contentColumn.implicitHeight
    spacing: Kirigami.Units.largeSpacing

    Rectangle {
        id: verticalBorder
        Layout.fillHeight: true

        implicitWidth: Kirigami.Units.smallSpacing
        color: root.replyAuthor.color
    }
    ColumnLayout {
        id: contentColumn
        implicitHeight: headerRow.implicitHeight + (root.replyComponentType != MessageComponentType.Other ? contentRepeater.itemAt(0).implicitHeight + spacing : 0)
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            id: headerRow
            implicitHeight: Math.max(replyAvatar.implicitHeight, replyName.implicitHeight)
            Layout.maximumWidth: root.maxContentWidth
            spacing: Kirigami.Units.largeSpacing

            KirigamiComponents.Avatar {
                id: replyAvatar

                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small

                source: root.replyAuthor.avatarSource
                name: root.replyAuthor.displayName
                color: root.replyAuthor.color
            }
            QQC2.Label {
                id: replyName
                Layout.fillWidth: true

                color: root.replyAuthor.color
                text: root.replyAuthor.displayName
                elide: Text.ElideRight
            }
        }
        Repeater {
            id: contentRepeater
            model: [root.replyComponentType]
            delegate: DelegateChooser {
                role: "modelData"

                DelegateChoice {
                    roleValue: MessageComponentType.Text
                    delegate: TextComponent {
                        display: root.replyDisplay
                        maxContentWidth: _private.availableContentWidth

                        onSelectedTextChanged: root.selectedTextChanged(selectedText)

                        HoverHandler {
                            enabled: !hoveredLink
                            cursorShape: Qt.PointingHandCursor
                        }
                        TapHandler {
                            enabled: !hoveredLink
                            acceptedButtons: Qt.LeftButton
                            onTapped: root.replyClicked(root.replyEventId)
                        }
                    }
                }
                DelegateChoice {
                    roleValue: MessageComponentType.Image
                    delegate: Image {
                        id: image
                        Layout.maximumWidth: mediaSizeHelper.currentSize.width
                        Layout.maximumHeight: mediaSizeHelper.currentSize.height
                        source: root?.replyMediaInfo.source ?? ""

                        MediaSizeHelper {
                            id: mediaSizeHelper
                            contentMaxWidth: _private.availableContentWidth
                            mediaWidth: root?.replyMediaInfo.width ?? -1
                            mediaHeight: root?.replyMediaInfo.height ?? -1
                        }
                    }
                }
                DelegateChoice {
                    roleValue: MessageComponentType.File
                    delegate: MimeComponent {
                        mimeIconSource: root.replyMediaInfo.mimeIcon
                        label: root.replyDisplay
                        subLabel: root.replyComponentType === DelegateType.File ? Format.formatByteSize(root.replyMediaInfo.size) : Format.formatDuration(root.replyMediaInfo.duration)
                    }
                }
                DelegateChoice {
                    roleValue: MessageComponentType.Video
                    delegate: MimeComponent {
                        mimeIconSource: root.replyMediaInfo.mimeIcon
                        label: root.replyDisplay
                        subLabel: root.replyComponentType === DelegateType.File ? Format.formatByteSize(root.replyMediaInfo.size) : Format.formatDuration(root.replyMediaInfo.duration)
                    }
                }
                DelegateChoice {
                    roleValue: MessageComponentType.Audio
                    delegate: MimeComponent {
                        mimeIconSource: root.replyMediaInfo.mimeIcon
                        label: root.replyDisplay
                        subLabel: root.replyComponentType === DelegateType.File ? Format.formatByteSize(root.replyMediaInfo.size) : Format.formatDuration(root.replyMediaInfo.duration)
                    }
                }
                DelegateChoice {
                    roleValue: MessageComponentType.Encrypted
                    delegate: TextComponent {
                        display: i18n("This message is encrypted and the sender has not shared the key with this device.")
                    }
                }
            }
        }
    }
    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: root.replyClicked(root.replyEventId)
    }
    QtObject {
        id: _private
        // The space available for the component after taking away the border
        readonly property real availableContentWidth: root.maxContentWidth - verticalBorder.implicitWidth - root.spacing
    }
}
