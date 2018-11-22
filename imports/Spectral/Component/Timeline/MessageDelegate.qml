import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Font 0.1

ColumnLayout {
    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool highlighted: !(sentByMe || eventType === "notice" )
    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isText: eventType === "notice" || eventType === "message"

    signal saveFileAs()
    signal openExternally()

    Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

    id: root

    spacing: 0

    Label {
        Layout.leftMargin: 48

        text: author.displayName

        visible: avatarVisible

        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
    }

    RowLayout {
        Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

        z: -5

        id: messageRow

        spacing: 4

        ImageItem {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignTop

            visible: avatarVisible
            hint: author.displayName
            source: author.paintable
        }

        Label {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignTop

            visible: !(sentByMe || avatarVisible)

            text: Qt.formatDateTime(time, "hh:mm")
            color: "#5B7480"

            font.pixelSize: 10
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }

        Control {
            Layout.maximumWidth: messageListView.width - (!sentByMe ? 32 + messageRow.spacing : 0) - 48

            topPadding: 8
            bottomPadding: 8
            leftPadding: 16
            rightPadding: 16

            background: Rectangle {
                color: sentByMe ? "#009DC2" : eventType === "notice" ? "#4285F4" : "#673AB7"
                radius: 18

                AutoMouseArea {
                    anchors.fill: parent

                    id: messageMouseArea

                    onSecondaryClicked: {
                        messageContextMenu.root = root
                        messageContextMenu.model = model
                        messageContextMenu.selectedText = contentLabel.selectedText
                        messageContextMenu.popup()
                    }
                }
            }

            contentItem: ColumnLayout {
                id: messageColumn

                spacing: 0

                TextEdit {
                    Layout.fillWidth: true

                    id: contentLabel

                    text: "<style>a{color: white;} .user-pill{color: white}</style>" + display

                    visible: isText
                    color: "white"

                    font.family: CommonFont.font.family
                    font.pixelSize: 14
                    selectByMouse: true
                    readOnly: true
                    wrapMode: Label.Wrap
                    selectedTextColor: highlighted ? Material.accent : "white"
                    selectionColor: highlighted ? "white" : Material.accent
                    textFormat: Text.RichText

                    onLinkActivated: Qt.openUrlExternally(link)

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
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
            }

            Component {
                id: imageComponent

                DownloadableContent {
                    width: messageImage.width
                    height: messageImage.height

                    id: downloadable

                    TimelineImage {
                        z: -4

                        id: messageImage

                        sourceSize: 128
                        source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)

                        onClicked: downloadAndOpen()
                    }

                    Component.onCompleted: {
                        root.saveFileAs.connect(saveFileAs)
                        root.openExternally.connect(downloadAndOpen)
                    }
                }
            }

            Component {
                id: fileComponent

                TimelineLabel {
                    Layout.fillWidth: true

                    id: downloadDelegate

                    text: "<b>File: </b>" + content.body
                    coloredBackground: highlighted

                    background: DownloadableContent {
                        id: downloadable

                        Component.onCompleted: {
                            root.saveFileAs.connect(saveFileAs)
                            root.openExternally.connect(downloadAndOpen)
                        }
                    }
                }
            }

            Component {
                id: audioComponent

                TimelineLabel {
                    id: downloadDelegate

                    text: content.info.duration / 1000 + '"'
                    coloredBackground: highlighted

                    MouseArea {
                        anchors.fill: parent

                        propagateComposedEvents: true

                        onClicked: {
                            if (downloadable.downloaded)
                                spectralController.playAudio(progressInfo.localPath)
                            else
                            {
                                playOnFinished = true
                                currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_") + ".tmp")
                            }
                        }
                    }

                    background: DownloadableContent {
                        id: downloadable

                        onDownloadedChanged: downloaded && playOnFinished ? spectralController.playAudio(progressInfo.localPath) : {}

                        Component.onCompleted: {
                            root.saveFileAs.connect(saveFileAs)
                            root.openExternally.connect(downloadAndOpen)
                        }
                    }
                }
            }
        }
    }
}
