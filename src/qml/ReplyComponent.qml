// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

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
Item {
    id: root

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
    required property var author

    /**
     * @brief The delegate type of the reply message.
     */
    required property int type

    /**
     * @brief The display text of the message.
     */
    required property string display

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
    required property var mediaInfo

    property real contentMaxWidth

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked()

    implicitWidth: mainLayout.implicitWidth
    implicitHeight: mainLayout.implicitHeight

    GridLayout {
        id: mainLayout
        anchors.fill: parent
        implicitHeight: Math.max(replyAvatar.implicitHeight, replyName.implicitHeight) + loader.height

        rows: 2
        columns: 3
        rowSpacing: Kirigami.Units.smallSpacing
        columnSpacing: Kirigami.Units.largeSpacing

        Rectangle {
            id: verticalBorder

            Layout.fillHeight: true
            Layout.rowSpan: 2

            implicitWidth: Kirigami.Units.smallSpacing
            color: root.author.color
        }
        KirigamiComponents.Avatar {
            id: replyAvatar

            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small

            source: root.author.avatarSource
            name: root.author.displayName
            color: root.author.color
        }
        QQC2.Label {
            id: replyName
            Layout.fillWidth: true

            color: root.author.color
            text: root.author.displayName
            elide: Text.ElideRight
        }
        Loader {
            id: loader

            Layout.fillWidth: true
            Layout.maximumHeight: loader.item && (root.type == DelegateType.Image || root.type == DelegateType.Sticker) ? loader.item.height : loader.item.implicitHeight
            Layout.columnSpan: 2

            sourceComponent: {
                switch (root.type) {
                    case DelegateType.Image:
                    case DelegateType.Sticker:
                        return imageComponent;
                    case DelegateType.Message:
                    case DelegateType.Notice:
                        return textComponent;
                    case DelegateType.File:
                    case DelegateType.Video:
                    case DelegateType.Audio:
                        return mimeComponent;
                    case DelegateType.Encrypted:
                        return encryptedComponent;
                    default:
                        return textComponent;
                }
            }
        }
        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: root.replyClicked()
        }
    }

    Component {
        id: textComponent
        RichLabel {
            textMessage: root.display

            HoverHandler {
                enabled: !hoveredLink
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                enabled: !hoveredLink
                acceptedButtons: Qt.LeftButton
                onTapped: root.replyClicked()
            }
        }
    }
    Component {
        id: imageComponent
        Image {
            id: image
            width: mediaSizeHelper.currentSize.width
            height: mediaSizeHelper.currentSize.height
            fillMode: Image.PreserveAspectFit
            source: root.mediaInfo.source

            MediaSizeHelper {
                id: mediaSizeHelper
                contentMaxWidth: root.contentMaxWidth - verticalBorder.width - mainLayout.columnSpacing
                mediaWidth: root.mediaInfo.width
                mediaHeight: root.mediaInfo.height
            }
        }
    }
    Component {
        id: mimeComponent
        MimeComponent {
            mimeIconSource: root.mediaInfo.mimeIcon
            label: root.display
            subLabel: root.type === DelegateType.File ? Format.formatByteSize(root.mediaInfo.size) : Format.formatDuration(root.mediaInfo.duration)
        }
    }
    Component {
        id: encryptedComponent
        RichLabel {
            textMessage: i18n("This message is encrypted and the sender has not shared the key with this device.")
            textFormat: Text.RichText
        }
    }
}
