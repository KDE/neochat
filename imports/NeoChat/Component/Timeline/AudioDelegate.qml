// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Qt.labs.platform 1.1 as Platform
import QtMultimedia 5.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0

Control {
    id: root

    Layout.fillWidth: true

    Audio {
        id: audio
        source: currentRoom.urlToMxcUrl(content.url)
        autoLoad: false
    }

    contentItem: ColumnLayout {
        RowLayout {
            ToolButton {
                icon.name: audio.playbackState == Audio.PlayingState ? "media-playback-pause" : "media-playback-start"

                onClicked: {
                    if (audio.playbackState == Audio.PlayingState) {
                        audio.pause()
                    } else {
                        audio.play()
                    }
                }
            }
            Label {
                text: model.display
            }
        }
        RowLayout {
            visible: audio.hasAudio
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            // Server doesn't support seeking, so use ProgressBar instead of Slider :(
            ProgressBar {
                from: 0
                to: audio.duration
                value: audio.position
            }

            Label {
                text: Controller.formatDuration(audio.position) + "/" + Controller.formatDuration(audio.duration)
            }
        }
    }
}
