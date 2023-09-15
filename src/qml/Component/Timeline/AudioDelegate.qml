// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtMultimedia

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

/**
 * @brief A timeline delegate for an audio message.
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
     *  - mimeType - The MIME type of the media (should be audio/xxx for this delegate).
     *  - mimeIcon - The MIME icon name (should be audio-xxx).
     *  - size - The file size in bytes.
     *  - duration - The length in seconds of the audio media.
     */
    required property var mediaInfo

    /**
     * @brief Whether the media has been downloaded.
     */
    readonly property bool downloaded: root.progressInfo && root.progressInfo.completed
    onDownloadedChanged: audio.play()

    onOpenContextMenu: RoomManager.viewEventMenu(eventId, author, delegateType, plainText, "", "", mediaInfo.mimeType, progressInfo)

    innerObject: ColumnLayout {
        Layout.fillWidth: true
        Layout.maximumWidth: root.contentMaxWidth

        MediaPlayer {
            id: audio
            source: root.progressInfo.localPath
        }

        states: [
            State {
                name: "notDownloaded"
                when: !root.progressInfo.completed && !root.progressInfo.active

                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-start"
                    onClicked: currentRoom.downloadFile(root.eventId)
                }
            },
            State {
                name: "downloading"
                when: root.progressInfo.active && !root.progressInfo.completed
                PropertyChanges {
                    target: downloadBar
                    visible: true
                }
                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-stop"
                    onClicked: {
                        currentRoom.cancelFileTransfer(root.eventId)
                    }
                }
            },
            State {
                name: "paused"
                when: root.progressInfo.completed && (audio.playbackState === Audio.StoppedState || audio.playbackState === Audio.PausedState)
                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-start"
                    onClicked: {
                        audio.play()
                    }
                }
            },
            State {
                name: "playing"
                when: root.progressInfo.completed && audio.playbackState === Audio.PlayingState

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
            value: root.progressInfo.progress
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
                visible: root.contentMaxWidth > Kirigami.Units.gridUnit * 12

                text: Controller.formatDuration(audio.position) + "/" + Controller.formatDuration(audio.duration)
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: Kirigami.Units.smallSpacing
            visible: audio.hasAudio && root.contentMaxWidth < Kirigami.Units.gridUnit * 12

            text: Controller.formatDuration(audio.position) + "/" + Controller.formatDuration(audio.duration)
        }
    }
}
