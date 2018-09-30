import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Spectral 0.1
import Spectral.Settings 0.1
import SortFilterProxyModel 0.2

import "../component"
import "../menu"
import "qrc:/js/md.js" as Markdown
import "qrc:/js/util.js" as Util

Item {
    property var currentRoom: null

    id: item

    MessageEventModel {
        id: messageEventModel
        room: currentRoom
    }

    RoomDrawer {
        width: Math.min(item.width * 0.7, 480)
        height: item.height

        id: roomDrawer

        room: currentRoom
    }

    Label {
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

            z: 10

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
                        image: spectralController.safeImage(currentRoom ? currentRoom.avatar : null)
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

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

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16

            id: messageListView

            displayMarginBeginning: 40
            displayMarginEnd: 40
            verticalLayoutDirection: ListView.BottomToTop
            spacing: 8

            flickDeceleration: 4096

            boundsBehavior: Flickable.DragOverBounds

            property int largestVisibleIndex: count > 0 ? indexAt(contentX, contentY + height - 1) : -1

            onContentYChanged: {
                // Check whether we're about to bump into the ceiling in 2 seconds
                var curVelocity = verticalVelocity // Snapshot the current speed
                if(curVelocity < 0 && contentY + curVelocity*2 < originY)
                {
                    // Request the amount of messages enough to scroll at this
                    // rate for 3 more seconds
                    currentRoom.getPreviousContent(20);
                }
            }

            onMovementEnded: currentRoom.saveViewport(sortedMessageEventModel.mapToSource(indexAt(contentX, contentY)), sortedMessageEventModel.mapToSource(largestVisibleIndex))

            displaced: Transition {
                NumberAnimation {
                    property: "y"; duration: 200
                    easing.type: Easing.OutQuad
                }
            }

            model: SortFilterProxyModel {
                id: sortedMessageEventModel

                sourceModel: messageEventModel

                filters: ExpressionFilter {
                    expression: marks !== 0x08 && marks !== 0x10
                }

                onModelReset: {
                    if (currentRoom)
                    {
                        var lastScrollPosition = mapFromSource(currentRoom.savedTopVisibleIndex())
                        if (lastScrollPosition === 0)
                            messageListView.positionViewAtBeginning()
                        else
                        {
                            console.log("Scrolling to position", lastScrollPosition)
                            messageListView.currentIndex = lastScrollPosition
                        }
                        if (messageListView.contentY < messageListView.originY + 10)
                            currentRoom.getPreviousContent(100)
                    }
                    console.log("Model timeline reset")
                }
            }

            delegate: ColumnLayout {
                width: parent.width

                id: delegateColumn

                spacing: 8

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: section !== aboveSection

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
            }

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

            Popup {
                property string sourceText

                x: (window.width - width) / 2
                y: (window.height - height) / 2
                width: 480

                id: sourceDialog

                parent: ApplicationWindow.overlay

                modal: true

                padding: 16

                closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside

                contentItem: ScrollView {
                    TextArea {
                        readOnly: true
                        selectByMouse: true

                        text: sourceDialog.sourceText
                    }
                }
            }

            Popup {
                property alias listModel: readMarkerListView.model

                x: (window.width - width) / 2
                y: (window.height - height) / 2
                width: 320

                id: readMarkerDialog

                parent: ApplicationWindow.overlay

                modal: true
                padding: 16

                closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside

                contentItem: ListView {
                    implicitHeight: Math.min(window.height - 64, readMarkerListView.contentHeight)

                    id: readMarkerListView

                    clip: true
                    boundsBehavior: Flickable.DragOverBounds

                    delegate: ItemDelegate {
                        width: parent.width
                        height: 48

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12

                            ImageItem {
                                Layout.preferredWidth: height
                                Layout.fillHeight: true

                                image: modelData.avatar
                                hint: modelData.displayName
                            }

                            Label {
                                Layout.fillWidth: true

                                text: modelData.displayName
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.leftMargin: 16
            Layout.rightMargin: 16

            color: Material.background

            Rectangle {
                anchors.verticalCenter: parent.top
                width:  parent.width
                height: 48

                color: MSettings.darkTheme ? "#303030" : "#fafafa"

                layer.enabled: true
                layer.effect: ElevationEffect {
                    elevation: 2
                }

                RowLayout {
                    anchors.fill: parent

                    spacing: 0

                    ItemDelegate {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        contentItem: MaterialIcon { icon: "\ue226" }

                        onClicked: currentRoom.chooseAndUploadFile()
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48

                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                        clip: true

                        TextArea {
                            property real progress: 0

                            id: inputField

                            wrapMode: Text.Wrap
                            placeholderText: "Send a Message"
                            leftPadding: 16
                            topPadding: 0
                            bottomPadding: 0
                            selectByMouse: true
                            verticalAlignment: TextEdit.AlignVCenter

                            text: currentRoom ? currentRoom.cachedInput : ""

                            background: Item {}

                            onTextChanged: {
                                timeoutTimer.restart()
                                repeatTimer.start()
                                currentRoom.cachedInput = text
                            }

                            ToolTip.visible: currentRoom && currentRoom.hasUsersTyping
                            ToolTip.text: currentRoom ? currentRoom.usersTyping : ""

                            Keys.onReturnPressed: {
                                if (event.modifiers & Qt.ShiftModifier) {
                                    inputField.insert(inputField.cursorPosition, "\n")
                                } else {
                                    inputField.postMessage(inputField.text)
                                    inputField.text = ""
                                }
                            }

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

                                var replyRe = new RegExp("^> <(.*)><(.*)> (.*)\n\n(.*)")
                                if (text.match(replyRe)) {
                                    var matches = text.match(replyRe)
                                    currentRoom.sendReply(matches[1], matches[2], matches[3], matches[4])
                                    return
                                }

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
                    }

                    ItemDelegate {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        id: emojiButton

                        contentItem: MaterialIcon { icon: "\ue24e" }

                        onClicked: emojiPicker.visible ? emojiPicker.close() : emojiPicker.open()

                        EmojiPicker {
                            x: window.width - 370
                            y: window.height - 400

                            width: 360
                            height: 320

                            id: emojiPicker

                            parent: ApplicationWindow.overlay

                            Material.elevation: 2

                            textArea: inputField
                        }
                    }
                }
            }
        }
    }
}
