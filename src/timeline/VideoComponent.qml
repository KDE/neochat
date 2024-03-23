// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtMultimedia
import Qt.labs.platform as Platform

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show a video from a message.
 */
Video {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The index of the delegate in the model.
     */
    required property var index

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The media info for the event.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media (should be image/xxx for this delegate).
     *  - mimeIcon - The MIME icon name (should be image-xxx).
     *  - size - The file size in bytes.
     *  - width - The width in pixels of the audio media.
     *  - height - The height in pixels of the audio media.
     *  - tempInfo - mediaInfo (with the same properties as this except no tempInfo) for a temporary image while the file downloads.
     */
    required property var mediaInfo

    /**
     * @brief FileTransferInfo for any downloading files.
     */
    required property var fileTransferInfo

    /**
     * @brief Whether the media has been downloaded.
     */
    readonly property bool downloaded: root.fileTransferInfo && root.fileTransferInfo.completed
    onDownloadedChanged: {
        if (downloaded) {
            root.source = root.fileTransferInfo.localPath;
        }
        if (downloaded && playOnFinished) {
            playSavedFile();
            playOnFinished = false;
        }
    }

    /**
     * @brief Whether the video should be played when downloaded.
     */
    property bool playOnFinished: false

    /**
     * @brief The timeline ListView this component is being used in.
     */
    required property ListView timeline

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.preferredWidth: mediaSizeHelper.currentSize.width
    Layout.preferredHeight: mediaSizeHelper.currentSize.height

    fillMode: VideoOutput.PreserveAspectFit

    states: [
        State {
            name: "notDownloaded"
            when: !root.fileTransferInfo.completed && !root.fileTransferInfo.active
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
            when: root.fileTransferInfo.active && !root.fileTransferInfo.completed
            PropertyChanges {
                target: downloadBar
                visible: true
            }
        },
        State {
            name: "paused"
            when: root.fileTransferInfo.completed && (root.playbackState === MediaPlayer.StoppedState || root.playbackState === MediaPlayer.PausedState)
            PropertyChanges {
                target: videoControls
                stateVisible: true
            }
            PropertyChanges {
                target: playButton
                icon.name: "media-playback-start"
                onClicked: {
                    MediaManager.startPlayback();
                    root.play();
                }
            }
        },
        State {
            name: "playing"
            when: root.fileTransferInfo.completed && root.playbackState === MediaPlayer.PlayingState
            PropertyChanges {
                target: videoControls
                stateVisible: true
            }
            PropertyChanges {
                target: playButton
                icon.name: "media-playback-pause"
                onClicked: root.pause()
            }
        }
    ]

    Connections {
        target: MediaManager
        function onPlaybackStarted() {
            if (root.playbackState === MediaPlayer.PlayingState) {
                root.pause();
            }
        }
    }

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
            to: root.fileTransferInfo.total
            value: root.fileTransferInfo.progress
        }
    }

    QQC2.Control {
        id: videoControls
        property bool stateVisible: false

        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.right: root.right
        visible: stateVisible && (videoHoverHandler.hovered || volumePopupHoverHandler.hovered || volumeSlider.hovered || videoControlTimer.running)

        contentItem: RowLayout {
            id: controlRow
            QQC2.ToolButton {
                id: playButton
            }
            QQC2.Slider {
                Layout.fillWidth: true
                from: 0
                to: root.duration
                value: root.position
                onMoved: root.seek(value)
            }
            QQC2.Label {
                text: Format.formatDuration(root.position) + "/" + Format.formatDuration(root.duration)
            }
            QQC2.ToolButton {
                id: volumeButton
                property var unmuteVolume: root.volume

                icon.name: root.volume <= 0 ? "player-volume-muted" : "player-volume"

                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.text: i18nc("@action:button", "Volume")

                onClicked: {
                    if (root.volume > 0) {
                        root.volume = 0;
                    } else {
                        if (unmuteVolume === 0) {
                            root.volume = 1;
                        } else {
                            root.volume = unmuteVolume;
                        }
                    }
                }
                onHoveredChanged: {
                    if (!hovered && (root.state === "paused" || root.state === "playing")) {
                        videoControlTimer.restart();
                        volumePopupTimer.restart();
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
                        value: root.volume
                        onMoved: {
                            root.volume = value;
                            volumeButton.unmuteVolume = value;
                        }
                        onHoveredChanged: {
                            if (!hovered && (root.state === "paused" || root.state === "playing")) {
                                rooteoControlTimer.restart();
                                volumePopupTimer.restart();
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
                            if (!hovered && (root.state === "paused" || root.state === "playing")) {
                                videoControlTimer.restart();
                                volumePopupTimer.restart();
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
                        root.timeline.interactive = false;
                        root.pause();
                        // We need to make sure the index is that of the MediaMessageFilterModel.
                        if (root.timeline.model instanceof MessageFilterModel) {
                            RoomManager.maximizeMedia(RoomManager.mediaMessageFilterModel.getRowForSourceItem(root.index));
                        } else {
                            RoomManager.maximizeMedia(root.index);
                        }
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
            if (!hovered && (root.state === "paused" || root.state === "playing")) {
                videoControlTimer.restart();
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
        onTapped: if (root.fileTransferInfo.completed) {
            if (root.playbackState == MediaPlayer.PlayingState) {
                root.pause();
            } else {
                MediaManager.startPlayback();
                root.play();
            }
        } else {
            root.downloadAndPlay();
        }
    }

    MediaSizeHelper {
        id: mediaSizeHelper
        contentMaxWidth: root.maxContentWidth
        mediaWidth: root.mediaInfo.width
        mediaHeight: root.mediaInfo.height
    }

    function downloadAndPlay() {
        if (root.downloaded) {
            playSavedFile();
        } else {
            playOnFinished = true;
            root.room.downloadFile(root.eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + root.room.fileNameToDownload(root.eventId));
        }
    }

    function playSavedFile() {
        root.stop();
        MediaManager.startPlayback();
        root.play();
    }
}
