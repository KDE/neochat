import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import Qt.labs.platform 1.0 as Platform

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Menu.Timeline 2.0
import Spectral.Effect 2.0
import Spectral.Font 0.1

RowLayout {
    readonly property bool avatarVisible: showAuthor && !sentByMe
    readonly property bool sentByMe: author === currentRoom.localUser

    property bool openOnFinished: false
    property bool playOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    property bool supportStreaming: true

    id: root

    spacing: 4

    z: -5

    onDownloadedChanged: {
        if (downloaded) {
            vid.source = progressInfo.localPath
        }

        if (downloaded && openOnFinished) {
            openSavedFile()
            openOnFinished = false
        }
        if (downloaded && playOnFinished) {
            playSavedFile()
            playOnFinished = false
        }
    }

    Avatar {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36
        Layout.alignment: Qt.AlignBottom

        visible: avatarVisible
        hint: author.displayName
        source: author.avatarMediaId
        color: author.color

        Component {
            id: userDetailDialog

            UserDetailDialog {}
        }

        RippleEffect {
            anchors.fill: parent

            circular: true

            onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom, "user": author}).open()
        }
    }

    Item {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36

        visible: !(sentByMe || avatarVisible)
    }

    Video {
        readonly property int maxWidth: messageListView.width - (!sentByMe ? 36 + root.spacing : 0) - 48

        Layout.preferredWidth: content.info.w > maxWidth ? maxWidth : content.info.w
        Layout.preferredHeight: content.info.w > maxWidth ? (content.info.h / content.info.w * maxWidth) : content.info.h

        id: vid

        loops: MediaPlayer.Infinite
        autoPlay: true

        fillMode: VideoOutput.PreserveAspectFit

        Component.onCompleted: {
            if (downloaded) {
                source = progressInfo.localPath
            } else {
                source = currentRoom.urlToMxcUrl(content.url)
            }
        }

        onDurationChanged: {
            if (!duration) {
                console.log("This server does not support media streaming")
                supportStreaming = false;
            }
        }

        onErrorChanged: {
            if (error != MediaPlayer.NoError) {
                console.log("This server does not support media streaming")
                supportStreaming = false;
            }
        }

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: vid.width
                height: vid.height
                radius: 18
            }
        }

        Image {
            readonly property bool isThumbnail: !(content.info.thumbnail_info == null || content.thumbnailMediaId == null)
            readonly property var info: isThumbnail ? content.info.thumbnail_info : content.info

            anchors.fill: parent

            visible: isThumbnail  && (vid.playbackState == MediaPlayer.StoppedState || vid.error != MediaPlayer.NoError)

            source: "image://mxc/" + (isThumbnail ? content.thumbnailMediaId : "")

            sourceSize.width: info.w
            sourceSize.height: info.h

            fillMode: Image.PreserveAspectCrop
        }

        Label {
            anchors.centerIn: parent

            visible: vid.playbackState != MediaPlayer.PlayingState
            color: "white"
            text: "Video"
            font.pixelSize: 16

            padding: 8

            background: Rectangle {
                radius: height / 2
                color: "black"
                opacity: 0.3
            }
        }

        Control {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8

            horizontalPadding: 8
            verticalPadding: 4

            contentItem: RowLayout {
                Label {
                    text: Qt.formatTime(time, "hh:mm AP")
                    color: "white"
                    font.pixelSize: 12
                }

                Label {
                    text: author.displayName
                    color: "white"
                    font.pixelSize: 12
                }
            }

            background: Rectangle {
                radius: height / 2
                color: "black"
                opacity: 0.3
            }
        }

        Rectangle {
            anchors.fill: parent

            visible: progressInfo.active && !downloaded

            color: "#BB000000"

            ProgressBar {
                anchors.centerIn: parent

                width: parent.width * 0.8

                from: 0
                to: progressInfo.total
                value: progressInfo.progress
            }
        }

        RippleEffect {
            anchors.fill: parent

            id: messageMouseArea

            onPrimaryClicked: {
                if (supportStreaming || progressInfo.completed) {
                    if (vid.playbackState == MediaPlayer.PlayingState) {
                        vid.pause()
                    } else {
                        vid.play()
                    }
                } else {
                    downloadAndPlay()
                }
            }

            onSecondaryClicked: {
                var contextMenu = imageDelegateContextMenu.createObject(ApplicationWindow.overlay)
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
                id: imageDelegateContextMenu

                FileDelegateContextMenu {}
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

    function downloadAndPlay()
    {
        if (downloaded) playSavedFile()
        else
        {
            playOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }

    function playSavedFile()
    {
        vid.stop()
        vid.play()
    }
}
