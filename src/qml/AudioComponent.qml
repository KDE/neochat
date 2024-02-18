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
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

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
    onDownloadedChanged: if (downloaded) {
        audio.play()
    }

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

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
                onClicked: root.room.downloadFile(root.eventId)
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
                    root.room.cancelFileTransfer(root.eventId)
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
                    audio.play()
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

    RowLayout {
        QQC2.ToolButton {
            id: playButton
        }
        QQC2.Label {
            text: root.display
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
    QQC2.ProgressBar {
        id: downloadBar
        visible: false
        Layout.fillWidth: true
        from: 0
        to: root.mediaInfo.size
        value: root.fileTransferInfo.progress
    }
    RowLayout {
        visible: audio.hasAudio

        QQC2.Slider {
            Layout.fillWidth: true
            from: 0
            to: audio.duration
            value: audio.position
            onMoved: audio.seek(value)
        }

        QQC2.Label {
            visible: root.maxContentWidth > Kirigami.Units.gridUnit * 12

            text: Format.formatDuration(audio.position) + "/" + Format.formatDuration(audio.duration)
        }
    }
    QQC2.Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: Kirigami.Units.smallSpacing
        visible: audio.hasAudio && root.maxContentWidth < Kirigami.Units.gridUnit * 12

        text: Format.formatDuration(audio.position) + "/" + Format.formatDuration(audio.duration)
    }
}
