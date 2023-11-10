// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A timeline delegate for an image message.
 *
 * @inherit MessageDelegate
 */
MessageDelegate {
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

    onOpenContextMenu: RoomManager.viewEventMenu(eventId, author, delegateType, plainText, "", "", mediaInfo.mimeType, progressInfo)

    bubbleContent: Item {
        id: imageContainer

        property var imageItem: root.mediaInfo.animated ? animatedImageLoader.item : imageLoader.item

        implicitWidth: mediaSizeHelper.currentSize.width
        implicitHeight: mediaSizeHelper.currentSize.height

        Loader {
            id: imageLoader

            anchors.fill: parent

            active: !root.mediaInfo.animated
            sourceComponent: Image {
                source: root.mediaInfo.source
                sourceSize.width: mediaSizeHelper.currentSize.width * Screen.devicePixelRatio
                sourceSize.height: mediaSizeHelper.currentSize.height * Screen.devicePixelRatio

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
            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
            onTapped: {
                imageContainer.QQC2.ToolTip.hide()
                if (root.mediaInfo.animated) {
                    imageContainer.imageItem.paused = true
                }
                // We need to make sure the index is that of the MediaMessageFilterModel.
                if (root.ListView.view.model instanceof MessageFilterModel) {
                    RoomManager.maximizeMedia(RoomManager.mediaMessageFilterModel.getRowForSourceItem(root.index))
                } else {
                    RoomManager.maximizeMedia(root.index)
                }
            }
        }

        function downloadAndOpen() {
            if (downloaded) {
                openSavedFile()
            } else {
                openOnFinished = true
                ListView.view.currentRoom.downloadFile(root.eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + ListView.view.currentRoom.fileNameToDownload(root.eventId))
            }
        }

        function openSavedFile() {
            if (UrlHelper.openUrl(root.progressInfo.localPath)) return;
            if (UrlHelper.openUrl(root.progressInfo.localDir)) return;
        }

        MediaSizeHelper {
            id: mediaSizeHelper
            contentMaxWidth: root.contentMaxWidth
            mediaWidth: root.mediaInfo.width
            mediaHeight: root.mediaInfo.height
        }
    }
}
