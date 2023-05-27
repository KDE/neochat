// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

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
     *  - object - The NeoChatUser object for the reply author.
     *
     * @sa NeoChatUser
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

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked()

    implicitWidth: mainLayout.implicitWidth
    implicitHeight: mainLayout.implicitHeight

    GridLayout {
        id: mainLayout

        anchors.fill: parent

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
        Kirigami.Avatar {
            id: replyAvatar

            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small

            source: root.author.avatarSource
            name: root.author.displayName || ""
            color: root.author.color
        }
        QQC2.Label {
            Layout.fillWidth: true

            color: root.author.color
            text: root.author.displayName
            elide: Text.ElideRight
        }
        Loader {
            id: loader

            Layout.fillWidth: true
            Layout.maximumHeight: loader.item && (root.type == MessageEventModel.Image || root.type == MessageEventModel.Sticker) ? loader.item.height : -1
            Layout.columnSpan: 2

            sourceComponent: {
                switch (root.type) {
                    case MessageEventModel.Image:
                    case MessageEventModel.Sticker:
                        return imageComponent;
                    case MessageEventModel.Message:
                    case MessageEventModel.Notice:
                        return textComponent;
                    case MessageEventModel.File:
                    case MessageEventModel.Video:
                    case MessageEventModel.Audio:
                        return mimeComponent;
                    case MessageEventModel.Encrypted:
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
            textFormat: Text.RichText

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

            property var imageWidth: {
                if (root.mediaInfo.width > 0) {
                    return root.mediaInfo.width;
                } else {
                    return sourceSize.width;
                }
            }
            property var imageHeight: {
                if (root.mediaInfo.height > 0) {
                    return root.mediaInfo.height;
                } else {
                    return sourceSize.height;
                }
            }

            readonly property var aspectRatio: imageWidth / imageHeight

            height: width / aspectRatio
            fillMode: Image.PreserveAspectFit
            source: root.mediaInfo.source
        }
    }
    Component {
        id: mimeComponent
        MimeComponent {
            mimeIconSource: root.mediaInfo.mimeIcon
            label: root.display
            subLabel: root.type === MessageEventModel.File ? Controller.formatByteSize(root.mediaInfo.size) : Controller.formatDuration(root.mediaInfo.duration)
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
