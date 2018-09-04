import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import Matrique 0.1
import Matrique.Settings 0.1

import "../component"
import "qrc:/js/md.js" as Markdown

Item {
    property var currentRoom: null

    id: item

    UserListModel {
        id: userListModel

        room: currentRoom
    }

    Drawer {
        id: roomDrawer

        width: Math.min(item.width * 0.7, 480)
        height: item.height

        edge: Qt.RightEdge
        interactive: false

        ToolButton {
            contentItem: MaterialIcon { icon: "\ue5c4" }

            onClicked: roomDrawer.close()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 32

            ImageStatus {
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64
                Layout.alignment: Qt.AlignHCenter

                source: currentRoom && currentRoom.avatarUrl != "" ? "image://mxc/" + currentRoom.avatarUrl : null
                displayText: currentRoom ? currentRoom.displayName : ""
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: currentRoom && currentRoom.id ? currentRoom.id : ""
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: currentRoom && currentRoom.canonicalAlias ? currentRoom.canonicalAlias : "No Canonical Alias"
            }

            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: roomNameField
                    Layout.fillWidth: true
                    text: currentRoom && currentRoom.name ? currentRoom.name : ""
                }

                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.preferredHeight: parent.height

                    contentItem: MaterialIcon { icon: "\ue5ca" }

                    onClicked: currentRoom.setName(roomNameField.text)
                }
            }

            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: roomTopicField

                    Layout.fillWidth: true
                    text: currentRoom && currentRoom.topic ? currentRoom.topic : ""
                }

                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.preferredHeight: parent.height

                    contentItem: MaterialIcon { icon: "\ue5ca" }

                    onClicked: currentRoom.setTopic(roomTopicField.text)
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true

                boundsBehavior: Flickable.DragOverBounds

                delegate: ItemDelegate {
                    width: parent.width
                    height: 48

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 12

                        ImageStatus {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true

                            source: avatar != "" ? "image://mxc/" + avatar : ""
                            displayText: name
                        }

                        Label {
                            Layout.fillWidth: true

                            text: name
                        }
                    }
                }

                model: userListModel

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    Pane {
        anchors.fill: parent
        padding: 0

        background: Label {
            anchors.centerIn: parent
            visible: !currentRoom
            text: "Please choose a room."
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            visible: currentRoom

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64

                color: Material.accent

                ItemDelegate {
                    anchors.fill: parent

                    onClicked: roomDrawer.open()

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12

                        spacing: 12

                        ImageStatus {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            source: currentRoom && currentRoom.avatarUrl != "" ? "image://mxc/" + currentRoom.avatarUrl : null
                            displayText: currentRoom ? currentRoom.displayName : ""
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignHCenter

                            visible: parent.width > 64

                            Label {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                text: currentRoom ? currentRoom.displayName : ""
                                color: "white"
                                font.pointSize: 12
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                text: currentRoom ? currentRoom.topic : ""
                                color: "white"
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 16

                z: -10

                spacing: 0

                ListView {
                    id: messageListView

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    displayMarginBeginning: 40
                    displayMarginEnd: 40
                    verticalLayoutDirection: ListView.BottomToTop
                    spacing: 8

                    boundsBehavior: Flickable.DragOverBounds
                    maximumFlickVelocity: 2048

                    model: MessageEventModel {
                        id: messageEventModel
                        room: currentRoom
                    }

                    delegate: ColumnLayout {
                        readonly property bool hidden: marks === EventStatus.Redacted || marks === EventStatus.Hidden

                        id: delegateColumn

                        width: parent.width
                        height: hidden ? -8 : undefined

                        spacing: 8

                        Label {
                            Layout.alignment: Qt.AlignHCenter

                            visible: section !== aboveSection && !hidden

                            text: section
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                            rightPadding: 8
                            topPadding: 4
                            bottomPadding: 4

                            background: Rectangle {
                                color: MSettings.darkTheme ? "#484848" : "grey"
                            }
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter

                            visible: readMarker === true && index !== 0

                            text: "And Now"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                            rightPadding: 8
                            topPadding: 4
                            bottomPadding: 4

                            background: Rectangle {
                                color: MSettings.darkTheme ? "#484848" : "grey"
                            }
                        }

                        MessageDelegate {
                            visible: eventType === "notice" || eventType === "message" || eventType === "image" || eventType === "video" || eventType === "audio" || eventType === "file"
                        }

                        StateDelegate {
                            Layout.maximumWidth: messageListView.width * 0.8
                            visible: eventType === "emote" || eventType === "state"
                        }
                    }

                    ScrollBar.vertical: messageListViewScrollBar

                    onAtYBeginningChanged: atYBeginning && currentRoom ? currentRoom.getPreviousContent(20) : {}
                    onAtYEndChanged: atYEnd && currentRoom ? currentRoom.markAllMessagesAsRead() : {}

                    RoundButton {
                        id: goTopFab
                        width: 64
                        height: 64
                        visible: !parent.atYEnd

                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        contentItem: MaterialIcon {
                            anchors.fill: parent

                            icon: "\ue313"
                            color: "white"
                        }

                        Material.background: Material.accent

                        onClicked: parent.positionViewAtBeginning()

                        Behavior on opacity { NumberAnimation { duration: 200 } }
                    }
                }

                ScrollBar {
                    id: messageListViewScrollBar

                    Layout.preferredWidth: 16
                    Layout.fillHeight: true
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                Layout.margins: 16

                spacing: 0

                ItemDelegate {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    contentItem: MaterialIcon { icon: "\ue226" }

                    onClicked: currentRoom.chooseAndUploadFile()
                }

                TextField {
                    property real progress: 0

                    id: inputField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    placeholderText: "Send a Message"
                    leftPadding: 16
                    topPadding: 0
                    bottomPadding: 0
                    selectByMouse: true

                    text: currentRoom ? currentRoom.cachedInput : ""

                    onTextChanged: {
                        timeoutTimer.restart()
                        repeatTimer.start()
                        currentRoom.cachedInput = text
                    }

                    Keys.onReturnPressed: {
                        if (inputField.text) {
                            inputField.postMessage(inputField.text)
                            inputField.text = ""
                        }
                    }

                    background: Rectangle {
                        color: MSettings.darkTheme ? "#282828" : "#eaeaea"
                    }

                    ToolTip.visible: currentRoom && currentRoom.hasUsersTyping
                    ToolTip.text: currentRoom ? currentRoom.usersTyping : ""

                    Timer {
                        id: timeoutTimer

                        repeat: false
                        interval: 2000
                        onTriggered: {
                            repeatTimer.stop()
                            currentRoom.sendTypingNotification(false)
                        }
                    }

                    Timer {
                        id: repeatTimer

                        repeat: true
                        interval: 5000
                        triggeredOnStart: true
                        onTriggered: currentRoom.sendTypingNotification(true)
                    }

                    function postMessage(text) {
                        if (text.trim().length === 0) { return }
                        if(!currentRoom) { return }

                        var PREFIX_ME = '/me '
                        var PREFIX_NOTICE = '/notice '
                        var PREFIX_RAINBOW = '/rainbow '
                        var PREFIX_HTML = '/html '
                        var PREFIX_MARKDOWN = '/md '

                        if (text.indexOf(PREFIX_ME) === 0) {
                            text = text.substr(PREFIX_ME.length)
                            currentRoom.postMessage(text, RoomMessageEvent.Emote)
                            return
                        }
                        if (text.indexOf(PREFIX_NOTICE) === 0) {
                            text = text.substr(PREFIX_NOTICE.length)
                            currentRoom.postMessage(text, RoomMessageEvent.Notice)
                            return
                        }
                        if (text.indexOf(PREFIX_RAINBOW) === 0) {
                            text = text.substr(PREFIX_RAINBOW.length)

                            var parsedText = ""
                            var rainbowColor = ["#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500", "#ffff00", "#d4ff00", "#aaff00", "#80ff00", "#55ff00", "#2bff00", "#00ff00", "#00ff2b", "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff", "#00d4ff", "#00aaff", "#007fff", "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff", "#aa00ff", "#d400ff", "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"]
                            for (var i = 0; i < text.length; i++) {
                                parsedText = parsedText + "<font color='" + rainbowColor[i % rainbowColor.length] + "'>" + text.charAt(i) + "</font>"
                            }
                            currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text)
                            return
                        }
                        if (text.indexOf(PREFIX_HTML) === 0) {
                            text = text.substr(PREFIX_HTML.length)
                            var re = new RegExp("<.*?>")
                            var plainText = text.replace(re, "")
                            currentRoom.postHtmlMessage(plainText, text, RoomMessageEvent.Text)
                            return
                        }
                        if (text.indexOf(PREFIX_MARKDOWN) === 0) {
                            text = text.substr(PREFIX_MARKDOWN.length)
                            var parsedText = Markdown.markdown_parser(text)
                            currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text)
                            return
                        }

                        currentRoom.postPlainText(text)
                    }
                }

                ItemDelegate {
                    id: emojiButton

                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    contentItem: MaterialIcon { icon: "\ue24e" }

                    background: Rectangle { color: MSettings.darkTheme ? "#282828" : "#eaeaea" }

                    onClicked: emojiPicker.visible ? emojiPicker.close() : emojiPicker.open()

                    EmojiPicker {
                        id: emojiPicker

                        parent: ApplicationWindow.overlay

                        x: window.width - 370
                        y: window.height - 440

                        width: 360
                        height: 360

                        textArea: inputField
                    }
                }
            }
        }
    }

    onCurrentRoomChanged: if (currentRoom && currentRoom.timelineSize === 0) currentRoom.getPreviousContent(20)
}
