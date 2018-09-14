import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Matrique 0.1
import Matrique.Settings 0.1
import "qrc:/js/util.js" as Util

RowLayout {
    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool highlighted: !(sentByMe || eventType === "notice" )
    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isText: eventType === "notice" || eventType === "message"

    signal saveFileAs()
    signal openExternally()

    z: -5

    id: messageRow

    Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

    spacing: 6

    ImageItem {
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignTop

        round: false
        visible: avatarVisible
        hint: author.displayName
        defaultColor: Util.stringToColor(author.displayName)
        image: author.avatar
    }

    Rectangle {
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignTop

        color: "transparent"
        visible: !(sentByMe || avatarVisible)
    }

    GenericBubble {
        Layout.maximumWidth: messageListView.width - (!sentByMe ? 40 + messageRow.spacing : 0)

        id: genericBubble

        highlighted: messageRow.highlighted
        colored: highlighted && (eventType === "notice" || highlight)

        contentItem: ColumnLayout {
            id: messageColumn

            spacing: 0

            AutoLabel {
                visible: messageRow.avatarVisible
                text: author.displayName
                Material.foreground: Material.accent
                coloredBackground: highlighted
                font.bold: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: inputField.insert(inputField.cursorPosition, author.displayName)
                }
            }

            AutoLabel {
                Layout.fillWidth: true

                text: display
                visible: isText
                coloredBackground: highlighted
            }

            Loader {
                sourceComponent: {
                    switch (eventType) {
                    case "image":
                        return imageComponent
                    case "file":
                        return fileComponent
                    case "audio":
                        return audioComponent
                    }
                }

                active: eventType === "image" || eventType === "file" || eventType === "audio"
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 8

                AutoLabel {
                    id: timeLabel

                    visible: Math.abs(time - aboveTime) > 600000 || index == 0
                    text: Qt.formatTime(time, "hh:mm")
                    coloredBackground: highlighted
                    Material.foreground: "grey"
                    font.pointSize: 8
                }

                MaterialIcon {
                    height: timeLabel.height

                    visible: userMarker.length > 0
                    icon: "\ue5ca"
                    color: highlighted ? "white": Material.foreground
                    font.pointSize: 12
                }
            }
        }

        Component {
            id: imageComponent

            DownloadableContent {
                width: messageImage.width
                height: messageImage.height

                id: downloadable

                AutoImage {
                    z: -4

                    id: messageImage

                    sourceSize: 128
                    source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)

                    onClicked: downloadAndOpen()
                }

                Component.onCompleted: {
                    messageRow.saveFileAs.connect(saveFileAs)
                    messageRow.openExternally.connect(downloadAndOpen)
                }
            }
        }

        Component {
            id: fileComponent

            AutoLabel {
                Layout.fillWidth: true

                id: downloadDelegate

                text: "<b>File: </b>" + content.body
                coloredBackground: highlighted

                background: DownloadableContent {
                    id: downloadable

                    Component.onCompleted: {
                        messageRow.saveFileAs.connect(saveFileAs)
                        messageRow.openExternally.connect(downloadAndOpen)
                    }
                }
            }
        }

        Component {
            id: audioComponent

            AutoLabel {
                id: downloadDelegate

                text: content.info.duration / 1000 + '"'
                coloredBackground: highlighted

                MouseArea {
                    anchors.fill: parent

                    propagateComposedEvents: true

                    onClicked: {
                        if (downloadable.downloaded)
                            matriqueController.playAudio(progressInfo.localPath)
                        else
                        {
                            playOnFinished = true
                            currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_") + ".tmp")
                        }
                    }
                }

                background: DownloadableContent {
                    id: downloadable

                    onDownloadedChanged: downloaded && playOnFinished ? matriqueController.playAudio(progressInfo.localPath) : {}

                    Component.onCompleted: {
                        messageRow.saveFileAs.connect(saveFileAs)
                        messageRow.openExternally.connect(downloadAndOpen)
                    }
                }
            }
        }
    }
}
