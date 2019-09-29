import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Menu.Timeline 2.0
import Spectral.Effect 2.0

ColumnLayout {
    readonly property bool avatarVisible: !sentByMe && showAuthor
    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool darkBackground: !sentByMe
    readonly property bool replyVisible: reply || false
    readonly property bool failed: marks === EventStatus.SendingFailed

    signal saveFileAs()
    signal openExternally()

    id: root

    z: -5

    spacing: 0

    RowLayout {
        Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

        id: messageRow

        spacing: 4

        Avatar {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignBottom

            visible: avatarVisible
            hint: author.displayName
            source: author.avatarMediaId

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
            Layout.maximumWidth: messageListView.width - (!sentByMe ? 36 + messageRow.spacing : 0) - 48
            Layout.minimumHeight: 36

            padding: 0

            background: AutoRectangle {
                readonly property int minorRadius: 2

                id: bubbleBackground

                color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : author.color
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
                        var contextMenu = messageDelegateContextMenu.createObject(ApplicationWindow.overlay)
                        contextMenu.viewSource.connect(function() {
                            messageSourceDialog.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
                        })
                        contextMenu.reply.connect(function() {
                            roomPanelInput.replyModel = model
                            roomPanelInput.isReply = true
                            roomPanelInput.focus()
                        })
                        contextMenu.redact.connect(function() {
                            currentRoom.redactEvent(eventId)
                        })
                        contextMenu.popup()
                    }


                    Component {
                        id: messageDelegateContextMenu

                        MessageDelegateContextMenu {}
                    }

                    Component {
                        id: messageSourceDialog

                        MessageSourceDialog {}
                    }
                }
            }

            contentItem: ColumnLayout {
                spacing: 0

                Control {
                    Layout.fillWidth: true

                    Layout.topMargin: 8
                    Layout.leftMargin: 8
                    Layout.rightMargin: 8

                    padding: 4
                    rightPadding: 12

                    visible: replyVisible

                    contentItem: RowLayout {
                        Avatar {
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28
                            Layout.alignment: Qt.AlignTop

                            source: replyVisible ? reply.author.avatarMediaId : ""
                            hint: replyVisible ? reply.author.displayName : "H"

                            RippleEffect {
                                anchors.fill: parent

                                circular: true

                                onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom, "user": reply.author}).open()
                            }
                        }

                        TextEdit {
                            Layout.fillWidth: true

                            color: !sentByMe ? MPalette.foreground : "white"
                            text: "<style>a{color: " + color + ";} .user-pill{}</style>" + (replyVisible ? reply.display : "")

                            font.family: window.font.family
                            selectByMouse: true
                            readOnly: true
                            wrapMode: Label.Wrap
                            selectedTextColor: darkBackground ? "white" : MPalette.accent
                            selectionColor: darkBackground ? MPalette.accent : "white"
                            textFormat: Text.RichText
                        }
                    }

                    background: Rectangle {
                        color: replyVisible && sentByMe ? reply.author.color : MPalette.background
                        radius: 18

                        AutoMouseArea {
                            anchors.fill: parent

                            onClicked: goToEvent(reply.eventId)
                        }
                    }
                }

                TextEdit {
                    Layout.fillWidth: true

                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.topMargin: 8
                    Layout.bottomMargin: 8

                    id: contentLabel

                    text: "<style>a{color: " + color + ";} .user-pill{}</style>" + display

                    color: darkBackground ? "white" : MPalette.foreground

                    font.family: window.font.family
                    font.pixelSize: (message.length === 2 && /(?:[\u2700-\u27bf]|(?:\ud83c[\udde6-\uddff]){2}|[\ud800-\udbff][\udc00-\udfff]|[\u0023-\u0039]\ufe0f?\u20e3|\u3299|\u3297|\u303d|\u3030|\u24c2|\ud83c[\udd70-\udd71]|\ud83c[\udd7e-\udd7f]|\ud83c\udd8e|\ud83c[\udd91-\udd9a]|\ud83c[\udde6-\uddff]|\ud83c[\ude01-\ude02]|\ud83c\ude1a|\ud83c\ude2f|\ud83c[\ude32-\ude3a]|\ud83c[\ude50-\ude51]|\u203c|\u2049|[\u25aa-\u25ab]|\u25b6|\u25c0|[\u25fb-\u25fe]|\u00a9|\u00ae|\u2122|\u2139|\ud83c\udc04|[\u2600-\u26FF]|\u2b05|\u2b06|\u2b07|\u2b1b|\u2b1c|\u2b50|\u2b55|\u231a|\u231b|\u2328|\u23cf|[\u23e9-\u23f3]|[\u23f8-\u23fa]|\ud83c\udccf|\u2934|\u2935|[\u2190-\u21ff])/g.test(message)) ? 48 : 14
                    selectByMouse: true
                    readOnly: true
                    wrapMode: Label.Wrap
                    selectedTextColor: darkBackground ? MPalette.accent : "white"
                    selectionColor: darkBackground ? "white" : MPalette.accent
                    textFormat: Text.RichText

                    onLinkActivated: {
                        if (link.startsWith("https://matrix.to/")) {
                            var result = link.replace(/\?.*/, "").match("https://matrix.to/#/(!.*:.*)/(\\$.*:.*)")
                            if (!result || result.length < 3) return
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
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                    }
                }

                ReactionDelegate {
                    Layout.fillWidth: true

                    Layout.topMargin: 0
                    Layout.bottomMargin: 8
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                }
            }
        }
    }

    RowLayout {
        Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
        Layout.leftMargin: sentByMe ? undefined : 36 + messageRow.spacing + 12
        Layout.rightMargin: sentByMe ? 12 : undefined
        Layout.bottomMargin: 4

        visible: showAuthor && !failed

        Label {
            visible: !sentByMe

            text: author.displayName
            color: MPalette.lighter
        }

        Label {
            text: Qt.formatTime(time, "hh:mm AP")
            color: MPalette.lighter
        }
    }

    RowLayout {
        Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
        Layout.leftMargin: sentByMe ? undefined : 36 + messageRow.spacing + 12
        Layout.rightMargin: sentByMe ? 12 : undefined
        Layout.bottomMargin: 4

        visible: failed

        Label {
            text: "Send failed:"
            color: MPalette.lighter
        }

        Label {
            text: "Resend"
            color: MPalette.lighter

            MouseArea {
                anchors.fill: parent

                onClicked: currentRoom.retryMessage(eventId)
            }
        }

        Label {
            text: "|"
            color: MPalette.lighter
        }

        Label {
            text: "Discard"
            color: MPalette.lighter

            MouseArea {
                anchors.fill: parent

                onClicked: currentRoom.discardMessage(eventId)
            }
        }
    }
}
