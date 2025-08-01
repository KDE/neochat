// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtMultimedia

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show audio from a message.
 */
ColumnLayout {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

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
    onDownloadedChanged: if (downloaded) {
        audio.play();
    }

    MediaPlayer {
        id: audio
        onErrorOccurred: (error, errorString) => console.warn("Audio playback error:" + error + errorString)
        audioOutput: AudioOutput {}
    }

    states: [
        State {
            name: "notDownloaded"
            when: !root.fileTransferInfo.completed && !root.fileTransferInfo.active

            PropertyChanges {
                target: playButton
                icon.name: "media-playback-start"
                onClicked: Message.room.downloadFile(root.eventId)
            }
        },
        State {
            name: "downloading"
            when: root.fileTransferInfo.active && !root.fileTransferInfo.completed
            PropertyChanges {
                target: downloadBar
                visible: true
            }
            PropertyChanges {
                target: playButton
                icon.name: "media-playback-stop"
                onClicked: {
                    Message.room.cancelFileTransfer(root.eventId);
                }
            }
        },
        State {
            name: "paused"
            when: root.fileTransferInfo.completed && (audio.playbackState === MediaPlayer.StoppedState || audio.playbackState === MediaPlayer.PausedState)
            PropertyChanges {
                target: playButton
                icon.name: "media-playback-start"
                onClicked: {
                    audio.source = root.fileTransferInfo.localPath;
                    MediaManager.startPlayback();
                    audio.play();
                }
            }
        },
        State {
            name: "playing"
            when: root.fileTransferInfo.completed && audio.playbackState === MediaPlayer.PlayingState

            PropertyChanges {
                target: playButton

                icon.name: "media-playback-pause"

                onClicked: audio.pause()
            }
        }
    ]

    Connections {
        target: MediaManager
        function onPlaybackStarted() {
            if (audio.playbackState === MediaPlayer.PlayingState) {
                audio.pause();
            }
        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.ToolButton {
            id: playButton
        }

        ColumnLayout {
            spacing: 0

            QQC2.Label {
                text: root.componentAttributes.filename
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: Format.formatDuration(root.componentAttributes.duration)
                color: Kirigami.Theme.disabledTextColor
                visible: !audio.hasAudio
                Layout.fillWidth: true
            }
        }
    }
    QQC2.ProgressBar {
        id: downloadBar
        visible: false
        Layout.fillWidth: true
        from: 0
        to: root.componentAttributes.size
        value: root.fileTransferInfo.progress
    }
    RowLayout {
        visible: audio.hasAudio

        QQC2.Slider {
            Layout.fillWidth: true
            from: 0
            to: audio.duration
            value: audio.position
            onMoved: audio.setPosition(value)
        }

        QQC2.Label {
            visible: root.Message.maxContentWidth > Kirigami.Units.gridUnit * 12

            text: Format.formatDuration(audio.position) + "/" + Format.formatDuration(audio.duration)
        }
    }
    QQC2.Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: Kirigami.Units.smallSpacing
        visible: audio.hasAudio && root.Message.maxContentWidth < Kirigami.Units.gridUnit * 12

        text: Format.formatDuration(audio.position) + "/" + Format.formatDuration(audio.duration)
    }
}
