// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

/**
 * @brief A timeline delegate for an image message.
 *
 * @inherit TimelineContainer
 */
TimelineContainer {
    id: root

    /**
     * @brief The media info for the event.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media (should be image/xxx for this delegate).
     *  - mimeIcon - The MIME icon name (should be image-xxx).
     *  - size - The file size in bytes.
     *  - width - The width in pixels of the audio media.
     *  - height - The height in pixels of the audio media.
     *  - tempInfo - mediaInfo (with the same properties as this except no tempInfo) for a temporary image while the file downloads.
     */
    required property var mediaInfo

    /**
     * @brief Whether the media has been downloaded.
     */
    readonly property bool downloaded: root.progressInfo && root.progressInfo.completed

    /**
     * @brief Whether the image should be automatically opened when downloaded.
     */
    property bool openOnFinished: false

    /**
     * @brief The maximum width of the image.
     */
    readonly property var maxWidth: Kirigami.Units.gridUnit * 30

    /**
     * @brief The maximum height of the image.
     */
    readonly property var maxHeight: Kirigami.Units.gridUnit * 30

    onOpenContextMenu: openFileContext(root)

    innerObject: Item {
        id: imageContainer

        property var imageWidth: {
            if (root.mediaInfo.width > 0) {
                return root.mediaInfo.width;
            } else {
                return root.contentMaxWidth;
            }
        }
        property var imageHeight: {
            if (root.mediaInfo.height > 0) {
                return root.mediaInfo.height;
            } else {
                // Default to a 16:9 placeholder
                return root.contentMaxWidth / 16 * 9;
            }
        }

        readonly property var aspectRatio: imageWidth / imageHeight
        /**
         * Whether the image should be limited by height or width.
         * We need to prevent excessively tall as well as excessively wide media.
         *
         * @note In the case of a tie the media is width limited.
         */
        readonly property bool limitWidth: imageWidth >= imageHeight

        readonly property size maxSize: {
            if (limitWidth) {
                let width = Math.min(root.contentMaxWidth, root.maxWidth);
                let height = width / aspectRatio;
                return Qt.size(width, height);
            } else {
                let height = Math.min(root.maxHeight, root.contentMaxWidth / aspectRatio);
                let width = height * aspectRatio;
                return Qt.size(width, height);
            }
        }

        Layout.maximumWidth: maxSize.width
        Layout.maximumHeight: maxSize.height
        Layout.preferredWidth: imageWidth
        Layout.preferredHeight: imageHeight

        property var imageItem: root.mediaInfo.animated ? animatedImageLoader.item : imageLoader.item

        implicitWidth: root.mediaInfo.animated ? animatedImageLoader.width : imageLoader.width
        implicitHeight: root.mediaInfo.animated ? animatedImageLoader.height : imageLoader.height

        Loader {
            id: imageLoader

            anchors.fill: parent

            active: !root.mediaInfo.animated
            sourceComponent: Image {
                source: root.mediaInfo.source
                sourceSize.width: imageContainer.maxSize.width * Screen.devicePixelRatio
                sourceSize.height: imageContainer.maxSize.height * Screen.devicePixelRatio

                fillMode: Image.PreserveAspectFit
            }
        }

        Loader {
            id: animatedImageLoader

            anchors.fill: parent

            active: root.mediaInfo.animated
            sourceComponent: AnimatedImage {
                source: root.mediaInfo.source

                fillMode: Image.PreserveAspectFit

                paused: !applicationWindow().active
            }
        }

        Image {
            anchors.fill: parent
            source: root.mediaInfo.tempInfo.source
            visible: imageContainer.imageItem.status !== Image.Ready
        }

        QQC2.ToolTip.text: root.display
        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        HoverHandler {
            id: hoverHandler
        }

        Rectangle {
            anchors.fill: parent

            visible: (root.progressInfo.active && !downloaded) || imageContainer.imageItem.status !== Image.Ready

            color: "#BB000000"

            QQC2.ProgressBar {
                anchors.centerIn: parent

                width: parent.width * 0.8

                from: 0
                to: root.progressInfo.total
                value: root.progressInfo.progress
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                imageContainer.QQC2.ToolTip.hide()
                if (root.mediaInfo.animated) {
                    imageContainer.imageItem.paused = true
                }
                root.ListView.view.interactive = false
                root.ListView.view.showMaximizedMedia(root.index)
            }
        }

        function downloadAndOpen() {
            if (downloaded) {
                openSavedFile()
            } else {
                openOnFinished = true
                currentRoom.downloadFile(root.eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(root.eventId))
            }
        }

        function openSavedFile() {
            if (UrlHelper.openUrl(root.progressInfo.localPath)) return;
            if (UrlHelper.openUrl(root.progressInfo.localDir)) return;
        }
    }
}
