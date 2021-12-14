// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Qt.labs.platform 1.1

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0


TimelineContainer {
    id: imageDelegate

    width: ListView.view.width

    onReplyClicked: ListView.view.goToEvent(eventID)
    hoverComponent: hoverActions

    innerObject: Image {
        id: img

        property var content: model.content
        readonly property bool isAnimated: contentType === "image/gif"

        property bool openOnFinished: false
        readonly property bool downloaded: progressInfo && progressInfo.completed

        readonly property bool isThumbnail: !(content.info.thumbnail_info == null || content.thumbnailMediaId == null)
        //    readonly property var info: isThumbnail ? content.info.thumbnail_info : content.info
        readonly property var info: content.info
        readonly property string mediaId: isThumbnail ? content.thumbnailMediaId : content.mediaId

        Layout.maximumWidth: imageDelegate.bubbleMaxWidth
        source: "image://mxc/" + mediaId

        Image {
            anchors.fill: parent
            source: content.info["xyz.amorgan.blurhash"] ? ("image://blurhash/" + content.info["xyz.amorgan.blurhash"]) : ""
            visible: parent.status !== Image.Ready
        }

        fillMode: Image.PreserveAspectFit

        ToolTip.text: display
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

        function saveFileAs() {
            var dialog = fileDialog.createObject(ApplicationWindow.overlay)
            dialog.open()
            dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
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
            acceptedButtons: Qt.RightButton
            onTapped: openFileContext(model, parent)
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onLongPressed: openFileContext(model, parent)
            onTapped: {
                fullScreenImage.createObject(parent, {
                    filename: eventId,
                    localPath: currentRoom.urlToDownload(eventId),
                    blurhash: model.content.info["xyz.amorgan.blurhash"],
                    imageWidth: content.info.w,
                    imageHeight: content.info.h
                }).showFullScreen();
            }
        }

        function downloadAndOpen()
        {
            if (downloaded) openSavedFile()
            else
            {
                openOnFinished = true
                currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
            }
        }

        function openSavedFile()
        {
            if (Qt.openUrlExternally(progressInfo.localPath)) return;
            if (Qt.openUrlExternally(progressInfo.localDir)) return;
        }
    }
}
