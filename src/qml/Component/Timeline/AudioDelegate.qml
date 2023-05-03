// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtMultimedia 5.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: audioDelegate

    onOpenContextMenu: openFileContext(model, audioDelegate)

    readonly property bool downloaded: model.progressInfo && model.progressInfo.completed
    onDownloadedChanged: audio.play()

    innerObject: ColumnLayout {
        Layout.fillWidth: true
        Layout.maximumWidth: audioDelegate.contentMaxWidth

        Audio {
            id: audio
            source: model.progressInfo.localPath
            autoLoad: false
        }

        states: [
            State {
                name: "notDownloaded"
                when: !model.progressInfo.completed && !model.progressInfo.active

                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-start"
                    onClicked: currentRoom.downloadFile(model.eventId)
                }
            },
            State {
                name: "downloading"
                when: model.progressInfo.active && !model.progressInfo.completed
                PropertyChanges {
                    target: downloadBar
                    visible: true
                }
                PropertyChanges {
                    target: playButton
                    icon.name: "media-playback-stop"
                    onClicked: {
                        currentRoom.cancelFileTransfer(model.eventId)
                    }
                }
            },
            State {
                name: "paused"
                when: model.progressInfo.completed && (audio.playbackState === Audio.StoppedState || audio.playbackState === Audio.PausedState)
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
                when: model.progressInfo.completed && audio.playbackState === Audio.PlayingState

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
                text: model.display
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
        QQC2.ProgressBar {
            id: downloadBar
            visible: false
            Layout.fillWidth: true
            from: 0
            to: model.mediaInfo.size
            value: model.progressInfo.progress
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
                visible: audioDelegate.contentMaxWidth > Kirigami.Units.gridUnit * 12

                text: Controller.formatDuration(audio.position) + "/" + Controller.formatDuration(audio.duration)
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: Kirigami.Units.smallSpacing
            visible: audio.hasAudio && audioDelegate.contentMaxWidth < Kirigami.Units.gridUnit * 12

            text: Controller.formatDuration(audio.position) + "/" + Controller.formatDuration(audio.duration)
        }
    }
}
