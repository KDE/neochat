/**
 * SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as Platform

import org.kde.neochat 1.0
import NeoChat.Setting 1.0

import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0

Image {
    readonly property bool isAnimated: contentType === "image/gif"

    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    readonly property bool isThumbnail: !(content.info.thumbnail_info == null || content.thumbnailMediaId == null)
    //    readonly property var info: isThumbnail ? content.info.thumbnail_info : content.info
    readonly property var info: content.info
    readonly property string mediaId: isThumbnail ? content.thumbnailMediaId : content.mediaId

    id: img

    source: "image://mxc/" + mediaId

    sourceSize.width: info.w
    sourceSize.height: info.h

    fillMode: Image.PreserveAspectFit

    Control {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8

        horizontalPadding: 8
        verticalPadding: 4

        contentItem: RowLayout {
            Label {
                text: Qt.formatTime(time)
                color: "white"
                font.pixelSize: 12
            }

            Label {
                text: author.displayName
                color: "white"
                font.pixelSize: 12
            }
        }

        background: Rectangle {
            radius: 2
            color: "black"
            opacity: 0.3
        }
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

    MouseArea {
        id: messageMouseArea

        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if(mouse.button === Qt.LeftButton) {
                fullScreenImage.createObject(parent, {"filename": eventId, "localPath": currentRoom.urlToDownload(eventId)}).showFullScreen()
            } else {
                openContextMenu()
            }
        }

        function openContextMenu() {
            var contextMenu = imageDelegateContextMenu.createObject(root, {'room': currentRoom, 'author': author});
            contextMenu.viewSource.connect(function() {
                messageSourceSheet.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
            })
            contextMenu.downloadAndOpen.connect(downloadAndOpen)
            contextMenu.saveFileAs.connect(saveFileAs)
            contextMenu.reply.connect(function() {
                roomPanelInput.replyModel = Object.assign({}, model)
                roomPanelInput.isReply = true
                roomPanelInput.focus()
            })
            contextMenu.redact.connect(function() {
                currentRoom.redactEvent(eventId)
            })
            contextMenu.popup()
        }

        Component {
            id: messageSourceSheet

            MessageSourceSheet {}
        }

        Component {
            id: openFolderDialog

            OpenFolderDialog {}
        }

        Component {
            id: imageDelegateContextMenu

            FileDelegateContextMenu {}
        }

        Component {
            id: fullScreenImage

            FullScreenImage {}
        }
    }

    function saveFileAs() {
        var folderDialog = openFolderDialog.createObject(ApplicationWindow.overlay)

        folderDialog.chosen.connect(function(path) {
            if (!path) return

            currentRoom.downloadFile(eventId, path + "/" + currentRoom.fileNameToDownload(eventId))
        })

        folderDialog.open()
    }

    function downloadAndOpen()
    {
        if (downloaded) openSavedFile()
        else
        {
            openOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }
}
