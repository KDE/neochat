/**
 * SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as Platform
import QtMultimedia 5.12
import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Setting 1.0

import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0
import NeoChat.Effect 1.0

Control {
    id: root

    Layout.fillWidth: true

    Audio {
        id: audio
        source: currentRoom.urlToMxcUrl(content.url)
        autoLoad: false
    }

    Kirigami.Action {
        id: saveFileAction
        onTriggered: {
            let contextMenu = fileDelegateContextMenu.createObject(root, {'room': currentRoom, 'author': author});
            contextMenu.viewSource.connect(function() {
                messagerSourceSheet.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
            })
            contextMenu.downloadAndOpen.connect(downloadAndOpen)
            contextMenu.saveFileAs.connect(saveFileAs)
            contextMenu.reply.connect(function() {
                roomPanelInput.replyModel = Object.assign({}, model)
                roomPanelInput.isReply = true
                roomPanelInput.focus()
            })
            contextMenu.redact.connect(function() {
                currentRoom.redactEvent(eventId)
            })
            contextMenu.popup()
        }
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
            // Server doesn't support seeking, so use ProgressBar instead of Slider :(
            ProgressBar {
                from: 0
                to: audio.duration
                value: audio.position
            }

            Label {
                text: humanSize(audio.position) + "/" + humanSize(audio.duration)
            }
        }
    }

    background: AutoMouseArea {
        anchors.fill: parent

        id: messageMouseArea

        onSecondaryClicked: saveFileAction.trigger()

        Component {
            id: messagerSourceSheet

            MessageSourceSheet {}
        }

        Component {
            id: openFolderDialog

            OpenFolderDialog {}
        }

        Component {
            id: fileDelegateContextMenu

            FileDelegateContextMenu {}
        }
    }

    function saveFileAs() {
        var folderDialog = openFolderDialog.createObject(ApplicationWindow.overlay)

        folderDialog.chosen.connect(function(path) {
            if (!path) return

            currentRoom.downloadFile(eventId, path + "/" + currentRoom.fileNameToDownload(eventId))
        })

        folderDialog.open()
    }

    function downloadAndOpen() {
        if (downloaded) {
            openSavedFile()
        } else {
            openOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile() {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }

    function humanSize(duration) {
        if (!duration) {
            return i18n("Unknown duration")
        }

        if (duration > 1000 * 60 * 60) {
            return new Date(duration).toLocaleTimeString(Qt.locale(), "hh:mm:ss")
        }

        return new Date(duration).toLocaleTimeString(Qt.locale(), "mm:ss")
    }
}
