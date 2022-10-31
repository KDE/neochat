// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtMultimedia 5.15
import Qt.labs.platform 1.1 as Platform

import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: videoDelegate

    property bool playOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    property bool supportStreaming: true
    readonly property int maxWidth: 1000 // TODO messageListView.width

    onOpenContextMenu: openFileContext(model, vid)

    onDownloadedChanged: {
        if (downloaded) {
            vid.source = progressInfo.localPath
        }

        if (downloaded && playOnFinished) {
            playSavedFile()
            playOnFinished = false
        }
    }

    innerObject: Video {
        id: vid

        Layout.maximumWidth: videoDelegate.contentMaxWidth
        Layout.fillWidth: true
        Layout.maximumHeight: Kirigami.Units.gridUnit * 15
        Layout.minimumHeight: Kirigami.Units.gridUnit * 5

        Layout.preferredWidth: (model.content.info.w === undefined || model.content.info.w > videoDelegate.maxWidth) ? videoDelegate.maxWidth : content.info.w
        Layout.preferredHeight: model.content.info.w === undefined ? (videoDelegate.maxWidth * 3 / 4) : (model.content.info.w > videoDelegate.maxWidth ? (model.content.info.h / model.content.info.w * videoDelegate.maxWidth) : model.content.info.h)

        loops: MediaPlayer.Infinite

        fillMode: VideoOutput.PreserveAspectFit

        onDurationChanged: {
            if (!duration) {
                vid.supportStreaming = false;
            }
        }

        onErrorChanged: {
            if (error != MediaPlayer.NoError) {
                vid.supportStreaming = false;
            }
        }

        Image {
            anchors.fill: parent

            visible: vid.playbackState == MediaPlayer.StoppedState || vid.error != MediaPlayer.NoError

            source: model.content.thumbnailMediaId ? "image://mxc/" + model.content.thumbnailMediaId : ""

            fillMode: Image.PreserveAspectFit
        }

        QQC2.Label {
            anchors.centerIn: parent

            visible: vid.playbackState == MediaPlayer.StoppedState || vid.error != MediaPlayer.NoError
            color: "white"
            text: i18n("Video")
            font.pixelSize: 16

            padding: 8

            background: Rectangle {
                radius: Kirigami.Units.smallSpacing
                color: "black"
                opacity: 0.3
            }
        }

        Rectangle {
            anchors.fill: parent

            visible: progressInfo.active && !videoDelegate.downloaded

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
            onTapped: if (vid.supportStreaming || progressInfo.completed) {
                if (vid.playbackState == MediaPlayer.PlayingState) {
                    vid.pause()
                } else {
                    vid.play()
                }
            } else {
                videoDelegate.downloadAndPlay()
            }
        }
    }

    function downloadAndPlay() {
        if (vid.downloaded) {
            playSavedFile()
        } else {
            playOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function playSavedFile() {
        vid.stop()
        vid.play()
    }
}
