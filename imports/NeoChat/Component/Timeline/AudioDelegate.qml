// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 5.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: audioDelegate

    onReplyClicked: ListView.view.goToEvent(eventID)

    onOpenContextMenu: openFileContext(model, audioDelegate)

    readonly property bool downloaded: model.progressInfo && model.progressInfo.completed
    onDownloadedChanged: audio.play()

    hoverComponent: hoverActions
    innerObject: Control {
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

        contentItem: ColumnLayout {
            RowLayout {
                ToolButton {
                    id: playButton
                }
                Label {
                    text: model.display
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
            ProgressBar {
                id: downloadBar
                visible: false
                Layout.fillWidth: true
                from: 0
                to: model.content.info.size
                value: model.progressInfo.progress
            }
            RowLayout {
                visible: audio.hasAudio
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing

                Slider {
                    from: 0
                    to: audio.duration
                    value: audio.position
                    onMoved: audio.seek(value)
                }

                Label {
                    text: Controller.formatDuration(audio.position) + "/" + Controller.formatDuration(audio.duration)
                }
            }
        }
    }
}
