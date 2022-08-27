// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0


TimelineContainer {
    id: imageDelegate

    onReplyClicked: ListView.view.goToEvent(eventID)
    hoverComponent: hoverActions

    onOpenContextMenu: openFileContext(model, imageDelegate)

    property var content: model.content
    readonly property bool isAnimated: contentType === "image/gif"

    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    readonly property bool isThumbnail: !(content.info.thumbnail_info == null || content.thumbnailMediaId == null)
    //    readonly property var info: isThumbnail ? content.info.thumbnail_info : content.info
    readonly property var info: content.info
    readonly property string mediaId: isThumbnail ? content.thumbnailMediaId : content.mediaId

    innerObject: Image {
        id: img

        Layout.maximumWidth: imageDelegate.contentMaxWidth
        Layout.maximumHeight: imageDelegate.contentMaxWidth / imageDelegate.info.w * imageDelegate.info.h
        Layout.preferredWidth: imageDelegate.info.w
        Layout.preferredHeight: imageDelegate.info.h
        source: model.mediaUrl

        Image {
            anchors.fill: parent
            source: content.info["xyz.amorgan.blurhash"] ? ("image://blurhash/" + content.info["xyz.amorgan.blurhash"]) : ""
            visible: parent.status !== Image.Ready
        }

        fillMode: Image.PreserveAspectFit

        ToolTip.text: model.display
        ToolTip.visible: hoverHandler.hovered

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
            onLongPressed: openFileContext(model, parent)
            onTapped: {
                fullScreenImage.createObject(parent, {
                    filename: eventId,
                    source: mediaUrl,
                    blurhash: model.content.info["xyz.amorgan.blurhash"],
                    imageWidth: content.info.w,
                    imageHeight: content.info.h,
                    modelData: model
                }).showFullScreen();
            }
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
