import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Font 0.1

ColumnLayout {
    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool sentByMe: author === currentRoom.localUser

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
        z: -5

        id: messageRow

        spacing: 4

        Avatar {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignTop

            visible: avatarVisible
            hint: author.displayName
            source: author.avatarUrl
        }

        Label {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignTop

            visible: !(sentByMe || avatarVisible)

            text: Qt.formatDateTime(time, "hh:mm")
            color: MPalette.lighter

            font.pixelSize: 10
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }

        Control {
            Layout.maximumWidth: messageListView.width - (!sentByMe ? 32 + messageRow.spacing : 0) - 48

            verticalPadding: 8
            horizontalPadding: 16

            background: Rectangle {
                color: sentByMe ? "#009DC2" : eventType === "notice" ? "#4285F4" : "#673AB7"
                radius: 18

                AutoMouseArea {
                    anchors.fill: parent

                    id: messageMouseArea

                    onSecondaryClicked: messageContextMenu.popup()

                    Menu {
                        readonly property string selectedText: contentLabel.selectedText

                        id: messageContextMenu

                        MenuItem {
                            text: "View Source"

                            onTriggered: {
                                sourceDialog.sourceText = toolTip
                                sourceDialog.open()
                            }
                        }
                        MenuItem {
                            text: "Reply"

                            onTriggered: {
                                roomPanelInput.replyUser = author
                                roomPanelInput.replyEventID = eventId
                                roomPanelInput.replyContent = messageContextMenu.selectedText || message
                                roomPanelInput.isReply = true
                                roomPanelInput.focus()
                            }
                        }
                        MenuItem {
                            text: "Redact"

                            onTriggered: currentRoom.redactEvent(eventId)
                        }
                    }
                }
            }

            contentItem: ColumnLayout {
                Control {
                    Layout.fillWidth: true

                    visible: replyEventId || ""

                    padding: 8

                    background: Item {
                        Rectangle {
                            anchors.leftMargin: 0
                            width: 2
                            height: parent.height

                            color: "white"
                        }
                        MouseArea {
                            anchors.fill: parent

                            onClicked: goToEvent(replyEventId)
                        }
                    }

                    contentItem: RowLayout {
                        spacing: 8

                        Avatar {
                            Layout.preferredWidth: 36
                            Layout.preferredHeight: 36
                            Layout.alignment: Qt.AlignTop

                            source: replyAuthor ? replyAuthor.avatarUrl : ""
                            hint: replyAuthor ? replyAuthor.displayName : "H"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: 0

                            Label {
                                Layout.fillWidth: true

                                color: "white"
                                text: replyAuthor ? replyAuthor.displayName : ""

                                font.pixelSize: 13
                                font.weight: Font.Medium
                            }

                            Label {
                                Layout.fillWidth: true

                                color: "white"
                                text: replyDisplay || ""

                                wrapMode: Label.Wrap
                                textFormat: Label.RichText
                            }
                        }
                    }
                }

                TextEdit {
                    Layout.fillWidth: true

                    id: contentLabel

                    text: "<style>a{color: white;} .user-pill{}</style>" + display

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
