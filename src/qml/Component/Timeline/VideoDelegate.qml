// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtMultimedia @QTMULTIMEDIA_MODULE_QML_VERSION@ 
import Qt.labs.platform 1.1 as Platform

import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0

/**
 * @brief A timeline delegate for a video message.
 *
 * @inherit TimelineContainer
 */
TimelineContainer {
    id: root

    /**
     * @brief The media info for the event.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media (should be video/xxx for this delegate).
     *  - mimeIcon - The MIME icon name (should be video-xxx).
     *  - size - The file size in bytes.
     *  - duration - The length in seconds of the audio media.
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
     * @brief Whether the video should be played when downloaded.
     */
    property bool playOnFinished: false

    /**
     * @brief The maximum width of the image.
     */
    readonly property var maxWidth: Kirigami.Units.gridUnit * 30

    /**
     * @brief The maximum height of the image.
     */
    readonly property var maxHeight: Kirigami.Units.gridUnit * 30

    onOpenContextMenu: openFileContext(vid)

    onDownloadedChanged: {
        if (downloaded) {
            vid.source = root.progressInfo.localPath
        }

        if (downloaded && playOnFinished) {
            playSavedFile()
            playOnFinished = false
        }
    }

    innerObject: Video {
        id: vid
        Layout.preferredWidth: mediaSizeHelper.currentSize.width
        Layout.preferredHeight: mediaSizeHelper.currentSize.height

        fillMode: VideoOutput.PreserveAspectFit
        @QTMULTIMEDIA_VIDEO_FLUSHMODE@

        states: [
            State {
                name: "notDownloaded"
                when: !root.progressInfo.completed && !root.progressInfo.active
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
                when: root.progressInfo.active && !root.progressInfo.completed
                PropertyChanges {
                    target: downloadBar
                    visible: true
                }
            },
            State {
                name: "paused"
                when: root.progressInfo.completed && (vid.playbackState === MediaPlayer.StoppedState || vid.playbackState === MediaPlayer.PausedState)
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
                when: root.progressInfo.completed && vid.playbackState === MediaPlayer.PlayingState
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

        Image {
            id: mediaThumbnail
            anchors.fill: parent
            visible: false

            source: root.mediaInfo.tempInfo.source
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
                to: root.progressInfo.total
                value: root.progressInfo.progress
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
                            root.ListView.view.interactive = false
                            vid.pause()
                            root.ListView.view.showMaximizedMedia(root.index)
                        }
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
            onTapped: if (root.progressInfo.completed) {
                if (vid.playbackState == MediaPlayer.PlayingState) {
                    vid.pause()
                } else {
                    vid.play()
                }
            } else {
                root.downloadAndPlay()
            }
        }

        MediaSizeHelper {
            id: mediaSizeHelper
            contentMaxWidth: root.contentMaxWidth
            mediaWidth: root.mediaInfo.width
            mediaHeight: root.mediaInfo.height
        }
    }

    function downloadAndPlay() {
        if (vid.downloaded) {
            playSavedFile()
        } else {
            playOnFinished = true
            currentRoom.downloadFile(root.eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(root.eventId))
        }
    }

    function playSavedFile() {
        vid.stop()
        vid.play()
    }
}
