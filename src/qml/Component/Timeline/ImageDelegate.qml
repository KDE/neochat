// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as Components

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
    readonly property var maxHeight: Kirigami.Units.gridUnit * 30

    innerObject: AnimatedImage {
        id: img

        property var imageWidth: {
            if (imageDelegate.info && imageDelegate.info.w && imageDelegate.info.w > 0) {
                return imageDelegate.info.w;
            } else if (sourceSize.width && sourceSize.width > 0) {
                return sourceSize.width;
            } else {
                return imageDelegate.contentMaxWidth;
            }
        }
        property var imageHeight: {
            if (imageDelegate.info && imageDelegate.info.h && imageDelegate.info.h > 0) {
                return imageDelegate.info.h;
            } else if (sourceSize.height && sourceSize.height > 0) {
                return sourceSize.height;
            } else {
                // Default to a 16:9 placeholder
                return imageDelegate.contentMaxWidth / 16 * 9;
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
                let width = Math.min(imageDelegate.contentMaxWidth, imageDelegate.maxWidth);
                let height = width / aspectRatio;
                return Qt.size(width, height);
            } else {
                let height = Math.min(imageDelegate.maxHeight, imageDelegate.contentMaxWidth / aspectRatio);
                let width = height * aspectRatio;
                return Qt.size(width, height);
            }
        }

        Layout.maximumWidth: maxSize.width
        Layout.maximumHeight: maxSize.height
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

        paused: !applicationWindow().active

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

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                img.QQC2.ToolTip.hide()
                img.paused = true
                imageDelegate.ListView.view.interactive = false
                var popup = maximizeImageComponent.createObject(QQC2.ApplicationWindow.overlay, {
                    modelData: model,
                })
                popup.closed.connect(() => {
                    imageDelegate.ListView.view.interactive = true
                    img.paused = false
                    popup.destroy()
                })
                popup.open()
            }
        }

        Component {
            id: maximizeImageComponent
            NeochatMaximizeComponent {}
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
