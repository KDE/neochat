import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import Matrique 0.1
import Matrique.Settings 0.1

import "qrc:/qml/component"
import "qrc:/qml/menu"
import "qrc:/js/md.js" as Markdown
import "qrc:/js/util.js" as Util

Item {
    property var currentRoom: null

    id: item

    RoomDrawer {
        width: Math.min(item.width * 0.7, 480)
        height: item.height

        id: roomDrawer

        room: currentRoom
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

                        ImageItem {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true

                            hint: currentRoom ? currentRoom.displayName : "No name"
                            defaultColor: Util.stringToColor(currentRoom ? currentRoom.displayName : "No name")
                            image: matriqueController.safeImage(currentRoom ? currentRoom.avatar : null)
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

                                text: currentRoom ? (currentRoom.topic).replace(/(\r\n\t|\n|\r\t)/gm,"") : ""
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
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: messageListView

                    clip: true
                    displayMarginBeginning: 40
                    displayMarginEnd: 40
                    verticalLayoutDirection: ListView.BottomToTop
                    spacing: 8

                    boundsBehavior: Flickable.DragOverBounds
                    flickDeceleration: 9001

                    cacheBuffer: 200

                    model: MessageEventModel {
                        id: messageEventModel
                        room: currentRoom
                    }

                    delegate: ColumnLayout {
                        readonly property bool hidden: marks === EventStatus.Redacted || marks === EventStatus.Hidden

                        width: parent.width
                        height: hidden ? -8 : undefined

                        id: delegateColumn

                        clip: true
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

                            background: Rectangle { color: MSettings.darkTheme ? "#484848" : "grey" }
                        }

                        MessageDelegate {
                            visible: eventType === "notice" || eventType === "message" || eventType === "image" || eventType === "video" || eventType === "audio" || eventType === "file"
                        }

                        StateDelegate {
                            Layout.maximumWidth: messageListView.width * 0.8

                            visible: eventType === "emote" || eventType === "state"
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter

                            visible: eventType === "other"

                            text: display
                            color: "grey"
                            font.italic: true
                        }
                    }

                    ScrollBar.vertical: messageListViewScrollBar

                    onAtYBeginningChanged: atYBeginning && currentRoom ? currentRoom.getPreviousContent(20) : {}
                    onAtYEndChanged: atYEnd && currentRoom ? currentRoom.markAllMessagesAsRead() : {}

                    RoundButton {
                        width: 64
                        height: 64

                        id: goTopFab

                        visible: !(parent.atYEnd || messageListView.moving)

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

                    MessageContextMenu { id: messageContextMenu }

                    Dialog {
                        property string sourceText

                        x: (window.width - width) / 2
                        y: (window.height - height) / 2
                        width: 480

                        id: sourceDialog

                        parent: ApplicationWindow.overlay

                        modal: true
                        standardButtons: Dialog.Ok

                        padding: 16

                        title: "View Source"

                        contentItem: ScrollView {
                            TextArea {
                                readOnly: true
                                selectByMouse: true

                                text: sourceDialog.sourceText
                            }
                        }
                    }
                }

                ScrollBar {
                    Layout.preferredWidth: 16
                    Layout.fillHeight: true

                    id: messageListViewScrollBar
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

                    Layout.fillWidth: true
                    Layout.preferredHeight: 48

                    id: inputField

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
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    id: emojiButton

                    contentItem: MaterialIcon { icon: "\ue24e" }

                    background: Rectangle { color: MSettings.darkTheme ? "#282828" : "#eaeaea" }

                    onClicked: emojiPicker.visible ? emojiPicker.close() : emojiPicker.open()

                    EmojiPicker {
                        x: window.width - 370
                        y: window.height - 440

                        width: 360
                        height: 360

                        id: emojiPicker

                        parent: ApplicationWindow.overlay

                        textArea: inputField
                    }
                }
            }
        }
    }

    onCurrentRoomChanged: if (currentRoom && currentRoom.timelineSize === 0) currentRoom.getPreviousContent(20)
}
