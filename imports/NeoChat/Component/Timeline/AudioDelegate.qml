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

import org.kde.neochat 0.1
import NeoChat.Setting 0.1

import NeoChat.Component 2.0
import NeoChat.Dialog 2.0
import NeoChat.Menu.Timeline 2.0
import NeoChat.Effect 2.0

RowLayout {
    readonly property bool avatarVisible: !sentByMe && showAuthor
    readonly property bool sentByMe: author.isLocalUser

    id: root

    spacing: 4

    z: -5

    Kirigami.Avatar {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36
        Layout.alignment: Qt.AlignBottom

        visible: avatarVisible
        name: author.displayName
        source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
        color: author.color

        Component {
            id: userDetailDialog

            UserDetailDialog {}
        }

        RippleEffect {
            anchors.fill: parent

            circular: true

            onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom, "user": author.object, "displayName": author.displayName, "avatarMediaId": author.avatarMediaId, "avatarUrl": author.avatarUrl}).open()
        }
    }

    Item {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36

        visible: !(sentByMe || avatarVisible)
    }

    Control {
        Layout.maximumWidth: messageListView.width - (!sentByMe ? 36 + root.spacing : 0) - 48

        padding: 12

        Audio {
            id: audio

            source: currentRoom.urlToMxcUrl(content.url)
        }

        contentItem: RowLayout {
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

            ColumnLayout {
                Label {
                    Layout.fillWidth: true

                    text: display
                    color: MPalette.foreground
                    wrapMode: Label.Wrap
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    font.capitalization: Font.AllUppercase
                }

                Label {
                    readonly property int duration: content.info.duration ?? audio.duration ?? 0

                    Layout.fillWidth: true

                    visible: duration

                    text: humanSize(duration)
                    color: MPalette.lighter
                    wrapMode: Label.Wrap
                }
            }
        }

        background: AutoRectangle {
            readonly property int minorRadius: 8

            id: bubbleBackground

            color: MPalette.background
            radius: 18

            topLeftVisible: !sentByMe && (bubbleShape == 3 || bubbleShape == 2)
            topRightVisible: sentByMe && (bubbleShape == 3 || bubbleShape == 2)
            bottomLeftVisible: !sentByMe && (bubbleShape == 1 || bubbleShape == 2)
            bottomRightVisible: sentByMe && (bubbleShape == 1 || bubbleShape == 2)

            topLeftRadius: minorRadius
            topRightRadius: minorRadius
            bottomLeftRadius: minorRadius
            bottomRightRadius: minorRadius

            AutoMouseArea {
                anchors.fill: parent

                id: messageMouseArea

                onSecondaryClicked: {
                    var contextMenu = fileDelegateContextMenu.createObject(root)
                    contextMenu.viewSource.connect(function() {
                        messageSourceDialog.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
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

                Component {
                    id: messageSourceDialog

                    MessageSourceDialog {}
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

    function downloadAndOpen()
    {
        if (downloaded) openSavedFile()
        else
        {
            openOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile() {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }

    function humanSize(duration) {
        if (!duration)
            return qsTr("Unknown", "Unknown duration")
        if (duration < 1000)
            return qsTr("An instant")
        duration = Math.round(duration / 100) / 10
        if (duration < 60)
            return qsTr("%1 sec.").arg(duration)
        duration = Math.round(duration / 6) / 10
        if (duration < 60)
            return qsTr("%1 min.").arg(duration)
        return qsTr("%1 hrs.").arg(Math.round(duration / 6) / 10)
    }
}
