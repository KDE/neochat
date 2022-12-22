// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: imageDelegate

    onOpenContextMenu: openFileContext(model, imageDelegate)

    property var content: model.content
    readonly property bool isAnimated: contentType === "image/gif"

    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    readonly property bool isThumbnail: !(content.info.thumbnail_info == null || content.thumbnailMediaId == null)
    //    readonly property var info: isThumbnail ? content.info.thumbnail_info : content.info
    readonly property var info: content.info
    readonly property string mediaId: isThumbnail ? content.thumbnailMediaId : content.mediaId

    readonly property var maxWidth: Kirigami.Units.gridUnit * 30

    innerObject: AnimatedImage {
        id: img

        property var imageWidth: {
            if (imageDelegate.info.w > 0) {
                return imageDelegate.info.w;
            } else if (sourceSize.width > 0) {
                return sourceSize.width;
            } else {
                return imageDelegate.contentMaxWidth;
            }
        }
        property var imageHeight: {
            if (imageDelegate.info.h > 0) {
                return imageDelegate.info.h;
            } else if (sourceSize.height > 0) {
                return sourceSize.height;
            } else {
                // Default to a 16:9 placeholder
                return imageDelegate.contentMaxWidth / 16 * 9;
            }
        }

        Layout.maximumWidth: Math.min(imageDelegate.contentMaxWidth, imageDelegate.maxWidth)
        Layout.maximumHeight: Math.min(imageDelegate.contentMaxWidth / imageWidth * imageHeight, imageDelegate.maxWidth / imageWidth * imageHeight)
        Layout.preferredWidth: imageWidth
        Layout.preferredHeight: imageHeight
        source: model.mediaUrl

        Image {
            anchors.fill: parent
            source: content.info["xyz.amorgan.blurhash"] ? ("image://blurhash/" + content.info["xyz.amorgan.blurhash"]) : ""
            visible: parent.status !== Image.Ready
        }

        fillMode: Image.PreserveAspectFit

        QQC2.ToolTip.text: model.display
        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        HoverHandler {
            id: hoverHandler
        }

        Rectangle {
            anchors.fill: parent

            visible: progressInfo.active && !downloaded

            color: "#BB000000"

            QQC2.ProgressBar {
                anchors.centerIn: parent

                width: parent.width * 0.8

                from: 0
                to: progressInfo.total
                value: progressInfo.progress
            }
        }

        Component {
            id: fileDialog

            FileDialog {
                fileMode: FileDialog.SaveFile
                folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
                onAccepted: {
                    currentRoom.downloadFile(eventId, file)
                }
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                img.QQC2.ToolTip.hide()
                img.paused = true
                fullScreenImage.open()
            }
        }

        FullScreenImage {
            id: fullScreenImage
            filename: eventId
            source: mediaUrl
            blurhash: model.content.info["xyz.amorgan.blurhash"]
            imageWidth: content.info.w
            imageHeight: content.info.h
            modelData: model

            onClosed: img.paused = false
        }

        function downloadAndOpen() {
            if (downloaded) {
                openSavedFile()
            } else {
                openOnFinished = true
                currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
            }
        }

        function openSavedFile() {
            if (UrlHelper.openUrl(progressInfo.localPath)) return;
            if (UrlHelper.openUrl(progressInfo.localDir)) return;
        }
    }
}
