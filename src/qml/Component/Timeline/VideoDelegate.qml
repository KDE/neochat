// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtMultimedia 5.15
import Qt.labs.platform 1.1 as Platform

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as Components

import org.kde.neochat 1.0

TimelineContainer {
    id: videoDelegate

    property bool playOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    property bool supportStreaming: true
    readonly property var maxWidth: Kirigami.Units.gridUnit * 30
    readonly property var maxHeight: Kirigami.Units.gridUnit * 30

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

        property var videoWidth: {
            if (model.mediaInfo.width > 0) {
                return model.mediaInfo.width;
            } else if (metaData.resolution && metaData.resolution.width) {
                return metaData.resolution.width;
            } else {
                return videoDelegate.contentMaxWidth;
            }
        }
        property var videoHeight: {
            if (model.mediaInfo.height > 0) {
                return model.mediaInfo.height;
            } else if (metaData.resolution && metaData.resolution.height) {
                return metaData.resolution.height;
            } else {
                // Default to a 16:9 placeholder
                return videoDelegate.contentMaxWidth / 16 * 9;
            }
        }

        readonly property var aspectRatio: videoWidth / videoHeight
        /**
         * Whether the video should be limited by height or width.
         * We need to prevent excessively tall as well as excessively wide media.
         *
         * @note In the case of a tie the media is width limited.
         */
        readonly property bool limitWidth: videoWidth >= videoHeight

        readonly property size maxSize: {
            if (limitWidth) {
                let width = Math.min(videoDelegate.contentMaxWidth, videoDelegate.maxWidth);
                let height = width / aspectRatio;
                return Qt.size(width, height);
            } else {
                let height = Math.min(videoDelegate.maxHeight, videoDelegate.contentMaxWidth / aspectRatio);
                let width = height * aspectRatio;
                return Qt.size(width, height);
            }
        }

        Layout.maximumWidth: maxSize.width
        Layout.maximumHeight: maxSize.height

        Layout.preferredWidth: videoWidth
        Layout.preferredHeight: videoHeight

        fillMode: VideoOutput.PreserveAspectFit
        flushMode: VideoOutput.FirstFrame

        states: [
            State {
                name: "notDownloaded"
                when: !model.progressInfo.completed && !model.progressInfo.active
                PropertyChanges {
                    target: noDownloadLabel
                    visible: true
                }
                PropertyChanges {
                    target: mediaThumbnail
                    visible: true
                }
            },
            State {
                name: "downloading"
                when: model.progressInfo.active && !model.progressInfo.completed
                PropertyChanges {
                    target: downloadBar
                    visible: true
                }
            },
            State {
                name: "paused"
                when: model.progressInfo.completed && (vid.playbackState === MediaPlayer.StoppedState || vid.playbackState === MediaPlayer.PausedState)
                PropertyChanges {
                    target: videoControls
                    stateVisible: true
                }
                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-start"
                    onClicked: vid.play()
                }
            },
            State {
                name: "playing"
                when: model.progressInfo.completed && vid.playbackState === MediaPlayer.PlayingState
                PropertyChanges {
                    target: videoControls
                    stateVisible: true
                }
                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-pause"
                    onClicked: vid.pause()
                }
            }
        ]

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
            id: mediaThumbnail
            anchors.fill: parent
            visible: false

            source: model.mediaInfo.thumbnailInfo.source
            fillMode: Image.PreserveAspectFit
        }

        QQC2.Label {
            id: noDownloadLabel
            anchors.centerIn: parent

            visible: false
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
            id: downloadBar
            anchors.fill: parent
            visible: false

            color: Kirigami.Theme.backgroundColor
            radius: Kirigami.Units.smallSpacing

            QQC2.ProgressBar {
                anchors.centerIn: parent

                width: parent.width * 0.8

                from: 0
                to: progressInfo.total
                value: progressInfo.progress
            }
        }

        QQC2.Control {
            id: videoControls
            property bool stateVisible: false

            anchors.bottom: vid.bottom
            anchors.left: vid.left
            anchors.right: vid.right
            visible: stateVisible && (videoHoverHandler.hovered || volumePopupHoverHandler.hovered || volumeSlider.hovered || videoControlTimer.running)

            contentItem: RowLayout {
                id: controlRow
                QQC2.ToolButton {
                    id: playButton
                }
                QQC2.Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: vid.duration
                    value: vid.position
                    onMoved: vid.seek(value)
                }
                QQC2.Label {
                    text: Controller.formatDuration(vid.position) + "/" + Controller.formatDuration(vid.duration)
                }
                QQC2.ToolButton {
                    id: volumeButton
                    property var unmuteVolume: vid.volume

                    icon.name: vid.volume <= 0 ? "player-volume-muted" : "player-volume"

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.timeout: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.text: i18nc("@action:button", "Volume")

                    onClicked: {
                        if (vid.volume > 0) {
                            vid.volume = 0
                        } else {
                            if (unmuteVolume === 0) {
                                vid.volume = 1
                            } else {
                                vid.volume = unmuteVolume
                            }
                        }
                    }
                    onHoveredChanged: {
                        if (!hovered && (vid.state === "paused" || vid.state === "playing")) {
                            videoControlTimer.restart()
                            volumePopupTimer.restart()
                        }
                    }

                    QQC2.Popup {
                        id: volumePopup
                        y: -height
                        width: volumeButton.width
                        visible: videoControls.stateVisible && (volumeButton.hovered || volumePopupHoverHandler.hovered || volumeSlider.hovered || volumePopupTimer.running)

                        focus: true
                        padding: Kirigami.Units.smallSpacing
                        closePolicy: QQC2.Popup.NoAutoClose

                        QQC2.Slider {
                            id: volumeSlider
                            anchors.centerIn: parent
                            implicitHeight: Kirigami.Units.gridUnit * 7
                            orientation: Qt.Vertical
                            padding: 0
                            from: 0
                            to: 1
                            value: vid.volume
                            onMoved: {
                                vid.volume = value
                                volumeButton.unmuteVolume = value
                            }
                            onHoveredChanged: {
                                if (!hovered && (vid.state === "paused" || vid.state === "playing")) {
                                    videoControlTimer.restart()
                                    volumePopupTimer.restart()
                                }
                            }
                        }
                        Timer {
                            id: volumePopupTimer
                            interval: 500
                        }
                        HoverHandler {
                            id: volumePopupHoverHandler
                            onHoveredChanged: {
                                if (!hovered && (vid.state === "paused" || vid.state === "playing")) {
                                    videoControlTimer.restart()
                                    volumePopupTimer.restart()
                                }
                            }
                        }
                        background: Kirigami.ShadowedRectangle {
                            radius: 4
                            color: Kirigami.Theme.backgroundColor
                            opacity: 0.8

                            property color borderColor: Kirigami.Theme.textColor
                            border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
                            border.width: 1

                            shadow.xOffset: 0
                            shadow.yOffset: 4
                            shadow.color: Qt.rgba(0, 0, 0, 0.3)
                            shadow.size: 8
                        }
                    }
                }
                QQC2.ToolButton {
                    id: maximizeButton
                    display: QQC2.AbstractButton.IconOnly

                    action: Kirigami.Action {
                        text: i18n("Maximize")
                        icon.name: "view-fullscreen"
                        onTriggered: {
                            videoDelegate.ListView.view.interactive = false
                            vid.pause()
                            var popup = maximizeVideoComponent.createObject(QQC2.ApplicationWindow.overlay, {
                                modelData: model,
                            })
                            popup.closed.connect(() => {
                                videoDelegate.ListView.view.interactive = true
                                popup.destroy()
                            })
                            popup.open()
                        }
                    }
                }
                Component {
                    id: maximizeVideoComponent
                    NeochatMaximizeComponent {}
                }
            }
            background: Kirigami.ShadowedRectangle {
                radius: 4
                color: Kirigami.Theme.backgroundColor
                opacity: 0.8

                property color borderColor: Kirigami.Theme.textColor
                border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
                border.width: 1

                shadow.xOffset: 0
                shadow.yOffset: 4
                shadow.color: Qt.rgba(0, 0, 0, 0.3)
                shadow.size: 8
            }
        }

        Timer {
            id: videoControlTimer
            interval: 1000
        }
        HoverHandler {
            id: videoHoverHandler
            onHoveredChanged: {
                if (!hovered && (vid.state === "paused" || vid.state === "playing")) {
                    videoControlTimer.restart()
                }
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
