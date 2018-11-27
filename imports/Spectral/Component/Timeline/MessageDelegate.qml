import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Font 0.1

ColumnLayout {
    readonly property int alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool sentByMe: author === currentRoom.localUser

    signal saveFileAs()
    signal openExternally()

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
                Control {
                    Layout.fillWidth: true

                    visible: replyEventId || ""

                    background: MouseArea {
                        onClicked: goToEvent(replyEventId)
                    }

                    contentItem: RowLayout {
                        spacing: 4

                        Rectangle {
                            Layout.preferredWidth: 2
                            Layout.fillHeight: true

                            color: "white"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: 0

                            Control {
                                padding: 4

                                contentItem: RowLayout {
                                    spacing: 4

                                    ImageItem {
                                        Layout.preferredWidth: 16
                                        Layout.preferredHeight: 16

                                        source: replyAuthor ? replyAuthor.paintable : null
                                    }

                                    Label {
                                        color: "white"
                                        text: replyAuthor ? replyAuthor.displayName : ""
                                    }
                                }

                                background: Rectangle {
                                    color: "black"
                                    opacity: 0.2
                                    radius: height / 2
                                }
                            }

                            Label {
                                Layout.fillWidth: true

                                text: "<style>a{color: white;} .user-pill{}</style>" + (replyDisplay ? replyDisplay.replace(/<mx-reply>.*<\/mx-reply>/g, "") : "")

                                color: "white"

                                wrapMode: Label.Wrap
                                textFormat: Label.RichText
                            }
                        }
                    }
                }

                TextEdit {
                    Layout.fillWidth: true

                    id: contentLabel

                    text: "<style>a{color: white;} .user-pill{}</style>" + (replyEventId ? display.replace(/<mx-reply>.*<\/mx-reply>/g, "") : display)

                    color: "white"

                    font.family: CommonFont.font.family
                    font.pixelSize: 14
                    selectByMouse: true
                    readOnly: true
                    wrapMode: Label.Wrap
                    selectedTextColor: Material.accent
                    selectionColor: "white"
                    textFormat: Text.RichText

                    onLinkActivated: {
                        if (link.startsWith("https://matrix.to/")) {
                            var result = link.replace(/\?.*/, "").match("https://matrix.to/#/(!.*:.*)/(\\$.*:.*)")
                            if (result.length < 3) return
                            if (result[1] != currentRoom.id) return
                            if (!result[2]) return
                            goToEvent(result[2])
                        } else {
                            Qt.openUrlExternally(link)
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }
            }
        }
    }
}
