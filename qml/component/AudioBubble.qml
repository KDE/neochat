import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtMultimedia 5.9
import Qt.labs.platform 1.0

AvatarContainer {
    readonly property var downloadAndOpen: downloadable.downloadAndOpen
    readonly property var saveFileAs: downloadable.saveFileAs

    property bool playOnFinished: false

    id: messageRow

    Audio { id: audioPlayer }

    DownloadableContent {
        id: downloadable

        width: downloadDelegate.width
        height: downloadDelegate.height

        TextDelegate {
            id: downloadDelegate

            maximumWidth: messageListView.width
            highlighted: !sentByMe
            timeLabelVisible: false
            authorLabelVisible: false

            displayText: content.info.duration / 1000 + '"'

            MouseArea {
                anchors.fill: parent

                propagateComposedEvents: true

                onClicked: {
                    if (downloadable.downloaded)
                        play()
                    else
                    {
                        playOnFinished = true
                        currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_") + ".tmp")
                    }
                }
            }
        }
        onDownloadedChanged: downloaded && playOnFinished ? play() : {}
    }

    function play() {
        audioPlayer.source = progressInfo.localPath
        audioPlayer.play()
    }
}
