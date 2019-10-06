import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as Platform
import QtMultimedia 5.12

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Menu.Timeline 2.0
import Spectral.Font 0.1
import Spectral.Effect 2.0

RowLayout {
    readonly property bool avatarVisible: !sentByMe && showAuthor
    readonly property bool sentByMe: author === currentRoom.localUser

    id: root

    spacing: 4

    z: -5

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

    Control {
        Layout.maximumWidth: messageListView.width - (!sentByMe ? 36 + root.spacing : 0) - 48

        padding: 12

        Audio {
            id: audio

            source: currentRoom.urlToMxcUrl(content.url)
        }

        contentItem: Label {
            text: content.info.duration || audio.duration || "Unknown audio"

            color: !sentByMe ? "white" : MPalette.foreground
        }

        background: AutoRectangle {
            readonly property int minorRadius: 8

            id: bubbleBackground

            color: sentByMe ? MPalette.background : author.color
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

                onPrimaryClicked: {
                    if (audio.playbackState === Audio.PlayingState) {
                        audio.stop()
                    } else {
                        audio.play()
                    }
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

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }
}
