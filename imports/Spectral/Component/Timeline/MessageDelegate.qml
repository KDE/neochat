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
    readonly property bool replyVisible: replyEventId || false

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

            verticalPadding: 8
            horizontalPadding: 16

            background: Rectangle {
                color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                radius: 18
                antialiasing: true

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left

                    width: parent.width / 2
                    height: parent.height / 2

                    visible: !sentByMe && (bubbleShape == 3 || bubbleShape == 2)

                    color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                    radius: 2
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right

                    width: parent.width / 2
                    height: parent.height / 2

                    visible: sentByMe && (bubbleShape == 3 || bubbleShape == 2)

                    color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                    radius: 2
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left

                    width: parent.width / 2
                    height: parent.height / 2

                    visible: !sentByMe && (bubbleShape == 1 || bubbleShape == 2)

                    color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                    radius: 2
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right

                    width: parent.width / 2
                    height: parent.height / 2

                    visible: sentByMe && (bubbleShape == 1 || bubbleShape == 2)

                    color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                    radius: 2
                }

                AutoMouseArea {
                    anchors.fill: parent

                    id: messageMouseArea

                    onSecondaryClicked: {
                        var contextMenu = messageDelegateContextMenu.createObject(ApplicationWindow.overlay)
                        contextMenu.viewSource.connect(function() {
                            messageSourceDialog.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
                        })
                        contextMenu.reply.connect(function() {
                            roomPanelInput.replyUser = author
                            roomPanelInput.replyEventID = eventId
                            roomPanelInput.replyContent = contentLabel.selectedText || message
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
                RowLayout {
                    Layout.fillWidth: true

                    visible: replyVisible

                    Avatar {
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        Layout.alignment: Qt.AlignTop

                        source: replyVisible ? replyAuthor.avatarMediaId : ""
                        hint: replyVisible ? replyAuthor.displayName : "H"

                        RippleEffect {
                            anchors.fill: parent

                            circular: true

                            onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom, "user": replyAuthor}).open()
                        }
                    }

                    Control {
                        Layout.fillWidth: true

                        visible: replyVisible

                        padding: 0

                        background: RippleEffect {
                            onClicked: goToEvent(replyEventId)
                        }

                        contentItem: Label {
                            color: darkBackground ? "white" : MPalette.lighter
                            text: "<style>a{color: " + (darkBackground ? "white" : MPalette.foreground) + ";} .user-pill{}</style>" + (replyDisplay || "")

                            wrapMode: Label.Wrap
                            textFormat: Label.RichText
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1

                    visible: replyVisible
                    color: darkBackground ? "white" : MPalette.lighter
                }

                TextEdit {
                    Layout.fillWidth: true

                    id: contentLabel

                    text: "<style>a{color: " + (darkBackground ? "white" : MPalette.foreground) + ";} .user-pill{}</style>" + display

                    color: darkBackground ? "white" : MPalette.foreground

                    font.family: window.font.family
                    font.pixelSize: 14
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
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
        Layout.leftMargin: sentByMe ? undefined : 36 + messageRow.spacing + 12
        Layout.rightMargin: sentByMe ? 12 : undefined
        Layout.bottomMargin: 4

        visible: showAuthor

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
}
