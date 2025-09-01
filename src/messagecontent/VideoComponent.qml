// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtMultimedia

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show a video from a message.
 */
Video {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The attributes of the component.
     */
    required property var componentAttributes

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

    Layout.preferredWidth: mediaSizeHelper.currentSize.width
    Layout.preferredHeight: mediaSizeHelper.currentSize.height

    fillMode: VideoOutput.PreserveAspectFit

    Component.onDestruction: root.stop()
    Component.onCompleted: {
        if (NeoChatConfig.hideImages && !Controller.isImageShown(root.eventId)) {
            root.state = "hidden";
        }
    }

    states: [
        State {
            name: "notDownloaded"
            when: !root.fileTransferInfo.completed && !root.fileTransferInfo.active
            PropertyChanges {
                videoLabel.visible: true
                mediaThumbnail.visible: true
            }
        },
        State {
            name: "downloading"
            when: root.fileTransferInfo.active && !root.fileTransferInfo.completed && (Controller.isImageShown(root.eventId) || !NeoChatConfig.hideImages)
            PropertyChanges {
                downloadBar.visible: true
                mediaThumbnail.visible: true
            }
        },
        State {
            name: "paused"
            when: root.fileTransferInfo.completed && root.playbackState === MediaPlayer.PausedState && (Controller.isImageShown(root.eventId) || !NeoChatConfig.hideImages)
            PropertyChanges {
                videoControls.stateVisible: true
                playButton.icon.name: "media-playback-start"
                playButton.onClicked: {
                    MediaManager.startPlayback();
                    root.play();
                }
            }
        },
        State {
            name: "playing"
            when: root.fileTransferInfo.completed && root.playbackState === MediaPlayer.PlayingState && (Controller.isImageShown(root.eventId) || !NeoChatConfig.hideImages)
            PropertyChanges {
                videoControls.stateVisible: true
                playButton.icon.name: "media-playback-pause"
                playButton.onClicked: root.pause()
            }
        },
        State {
            name: "stopped"
            when: root.fileTransferInfo.completed && root.playbackState === MediaPlayer.StoppedState && (Controller.isImageShown(root.eventId) || !NeoChatConfig.hideImages)
            PropertyChanges {
                videoControls.stateVisible: true
                mediaThumbnail.visible: true
                videoLabel.visible: true
                playButton.icon.name: "media-playback-start"
                playButton.onClicked: {
                    MediaManager.startPlayback();
                    root.play();
                }
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                mediaThumbnail.visible: false
                videoControls.visible: false
                hidden.visible: true
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

    QQC2.Button {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Kirigami.Units.smallSpacing
        visible: root.state !== "hidden"
        icon.name: "view-hidden"
        text: i18nc("@action:button", "Hide Image")
        display: QQC2.Button.IconOnly
        z: 10
        onClicked: {
            root.state = "hidden"
            Controller.markImageHidden(root.eventId)
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    Image {
        id: mediaThumbnail
        anchors.fill: parent
        visible: false

        source: visible ? root.componentAttributes.tempInfo.source : ""
        fillMode: Image.PreserveAspectFit
    }

    QQC2.Label {
        id: videoLabel
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

        color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.25)

        QQC2.ProgressBar {
            anchors.centerIn: parent

            width: parent.width * 0.8

            from: 0
            to: root.fileTransferInfo.total
            value: root.fileTransferInfo.progress
        }
    }

    Rectangle {
        id: hidden
        anchors.fill: parent

        visible: false
        color: "#BB000000"

        QQC2.Button {
            anchors.centerIn: parent
            text: i18nc("@action:button", "Show Video")
            onClicked: {
                root.state = "notDownloaded";
                Controller.markImageShown(root.eventId);
            }
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
                    if (!hovered && (root.state === "paused" || root.state === "stopped" || root.state === "playing")) {
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
                            if (!hovered && (root.state === "paused" || root.state === "stopped" || root.state === "playing")) {
                                videoControlTimer.restart();
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
                            if (!hovered && (root.state === "paused" || root.state === "stopped" || root.state === "playing")) {
                                videoControlTimer.restart();
                                volumePopupTimer.restart();
                            }
                        }
                    }
                    background: Kirigami.ShadowedRectangle {
                        radius: Kirigami.Units.cornerRadius
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

                text: i18nc("@action:button", "Maximize")
                icon.name: "view-fullscreen"
                onClicked: {
                    root.Message.timeline.interactive = false;
                    root.pause();
                    RoomManager.maximizeMedia(root.eventId);
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }
        background: Kirigami.ShadowedRectangle {
            radius: Kirigami.Units.cornerRadius
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
            if (!hovered && (root.state === "paused" || root.state === "stopped" || root.state === "playing")) {
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
        contentMaxWidth: root.Message.maxContentWidth
        mediaWidth: root.componentAttributes.width
        mediaHeight: root.componentAttributes.height
    }

    function downloadAndPlay() {
        if (root.downloaded) {
            playSavedFile();
        } else {
            playOnFinished = true;
            Message.room.downloadFile(root.eventId, Core.StandardPaths.writableLocation(Core.StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + Message.room.fileNameToDownload(root.eventId));
        }
    }

    function playSavedFile() {
        root.stop();
        MediaManager.startPlayback();
        root.play();
    }
}
