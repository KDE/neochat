import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Matrique 0.1
import Matrique.Settings 0.1

RowLayout {
    readonly property bool avatarVisible: !(sentByMe || (aboveAuthor === author && section === aboveSection))
    readonly property bool highlighted: !sentByMe
    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isText: eventType === "notice" || eventType === "message"

    signal saveFileAs()
    signal openExternally()

    id: messageRow

    z: -5

    Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

    spacing: 6

    ImageStatus {
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignTop

        round: false
        visible: avatarVisible
        source: author.avatarUrl != "" ? "image://mxc/" + author.avatarUrl : null
        displayText: author.displayName
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

        highlighted: !sentByMe
        colored: highlighted && eventType === "notice"

        contentItem: ColumnLayout {
            id: messageColumn

            spacing: 0

            AutoLabel {
                visible: messageRow.avatarVisible
                text: author.displayName
                Material.foreground: Material.accent
                coloredBackground: highlighted
                font.bold: true
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

            AutoLabel {
                Layout.alignment: Qt.AlignRight
                visible: Math.abs(time - aboveTime) > 600000 || index == 0
                text: Qt.formatTime(time, "hh:mm")
                coloredBackground: highlighted
                Material.foreground: "grey"
                font.pointSize: 8
            }
        }

        Component {
            id: imageComponent

            DownloadableContent {
                id: downloadable

                width: messageImage.width
                height: messageImage.height

                AutoImage {
                    id: messageImage
                    z: -4
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
