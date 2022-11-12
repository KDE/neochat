// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
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

        Layout.maximumWidth: Math.min(imageDelegate.contentMaxWidth, imageDelegate.maxWidth)
        Layout.maximumHeight: Math.min(imageDelegate.contentMaxWidth / sourceSize.width * sourceSize.height, imageDelegate.maxWidth / sourceSize.width * sourceSize.height)
        Layout.preferredWidth: imageDelegate.info.w > 0 ? imageDelegate.info.w : sourceSize.width
        Layout.preferredHeight: imageDelegate.info.h > 0 ? imageDelegate.info.h : sourceSize.height
        source: model.mediaUrl

        Image {
            anchors.fill: parent
            source: content.info["xyz.amorgan.blurhash"] ? ("image://blurhash/" + content.info["xyz.amorgan.blurhash"]) : ""
            visible: parent.status !== Image.Ready
        }

        fillMode: Image.PreserveAspectFit

        ToolTip.text: model.display
        ToolTip.visible: hoverHandler.hovered
        ToolTip.delay: Kirigami.Units.toolTipDelay

        HoverHandler {
            id: hoverHandler
        }

        Rectangle {
            anchors.fill: parent

            visible: progressInfo.active && !downloaded

            color: "#BB000000"

            ProgressBar {
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
                img.ToolTip.hide()
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
