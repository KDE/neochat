/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import Qt.labs.platform 1.0 as Platform
import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0

import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0
import NeoChat.Effect 1.0

Video {
    id: vid

    property bool openOnFinished: false
    property bool playOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    property bool supportStreaming: true

    onDownloadedChanged: {
        if (downloaded) {
            vid.source = progressInfo.localPath
        }

        if (downloaded && openOnFinished) {
            openSavedFile()
            openOnFinished = false
        }
        if (downloaded && playOnFinished) {
            playSavedFile()
            playOnFinished = false
        }
    }


    readonly property int maxWidth: 1000 // TODO messageListView.width

    Layout.preferredWidth: content.info.w > maxWidth ? maxWidth : content.info.w
    Layout.preferredHeight: content.info.w > maxWidth ? (content.info.h / content.info.w * maxWidth) : content.info.h

    loops: MediaPlayer.Infinite

    fillMode: VideoOutput.PreserveAspectFit

    Component.onCompleted: {
        if (downloaded) {
            source = progressInfo.localPath
        } else {
            source = currentRoom.urlToMxcUrl(content.url)
        }
    }

    onDurationChanged: {
        if (!duration) {
            supportStreaming = false;
        }
    }

    onErrorChanged: {
        if (error != MediaPlayer.NoError) {
            supportStreaming = false;
        }
    }

    Image {
        readonly property bool isThumbnail: content.info.thumbnail_info && content.thumbnailMediaId
        readonly property var info: isThumbnail ? content.info.thumbnail_info : content.info

        anchors.fill: parent

        visible: isThumbnail  && (vid.playbackState == MediaPlayer.StoppedState || vid.error != MediaPlayer.NoError)

        source: "image://mxc/" + (isThumbnail ? content.thumbnailMediaId : "")

        sourceSize.width: info.w
        sourceSize.height: info.h

        fillMode: Image.PreserveAspectFit
    }

    Label {
        anchors.centerIn: parent

        visible: vid.playbackState == MediaPlayer.StoppedState || vid.error != MediaPlayer.NoError
        color: "white"
        text: "Video"
        font.pixelSize: 16

        padding: 8

        background: Rectangle {
            radius: Kirigami.Units.smallSpacing
            color: "black"
            opacity: 0.3
        }
    }

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
            radius: height / 2
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

    AutoMouseArea {
        anchors.fill: parent

        id: messageMouseArea

        onPrimaryClicked: {
            if (supportStreaming || progressInfo.completed) {
                if (vid.playbackState == MediaPlayer.PlayingState) {
                    vid.pause()
                } else {
                    vid.play()
                }
            } else {
                downloadAndPlay()
            }
        }

        onSecondaryClicked: {
            var contextMenu = imageDelegateContextMenu.createObject(vid)
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

    function downloadAndPlay()
    {
        if (downloaded) playSavedFile()
        else
        {
            playOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }

    function playSavedFile()
    {
        vid.stop()
        vid.play()
    }
}
