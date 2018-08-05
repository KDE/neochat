import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import Matrique 0.1
import "qrc:/qml/component"
import "qrc:/js/md.js" as Markdown

Item {
    id: item
    property var currentRoom

    Pane {
        anchors.fill: parent
        padding: 0

        background: Item {
            anchors.fill: parent
            visible: !currentRoom
            Pane { anchors.fill: parent }

            Label {
                text: "Please choose a room."
                anchors.centerIn: parent
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            visible: currentRoom

            Pane {
                padding: 16

                Layout.fillWidth: true
                Layout.preferredHeight: 80

                background: Rectangle { color: Material.theme == Material.Light ? "#eaeaea" : "#242424" }

                RowLayout {
                    anchors.fill: parent
                    spacing: 16

                    ImageStatus {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true
                        source: currentRoom && currentRoom.avatarUrl != "" ? "image://mxc/" + currentRoom.avatarUrl : null
                        displayText: currentRoom ? currentRoom.displayName : ""
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Label {
                            Layout.fillWidth: true
                            text: currentRoom ? currentRoom.displayName : ""
                            font.pointSize: 16
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: currentRoom ? currentRoom.topic : ""
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
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
                    displayMarginBeginning: 40
                    displayMarginEnd: 40
                    verticalLayoutDirection: ListView.BottomToTop
                    spacing: 8

                    boundsBehavior: Flickable.DragOverBounds

                    model: MessageEventModel{
                        id: messageEventModel
                        room: currentRoom
                    }

                    delegate: Column {
                        width: parent.width
                        spacing: 8

                        RowLayout {
                            readonly property bool sectionVisible: section !== aboveSection

                            width: parent.width * 0.8
                            visible: sectionVisible
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                height:2
                                color: Material.accent
                            }

                            Label {
                                text: section
                                color: Material.accent
                                verticalAlignment: Text.AlignVCenter
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height:2
                                color: Material.accent
                            }
                        }

                        MessageDelegate {}
                    }

                    ScrollBar.vertical: messageListViewScrollBar

                    onAtYBeginningChanged: atYBeginning && currentRoom ? currentRoom.getPreviousContent(50) : {}
                    onAtYEndChanged: atYEnd && currentRoom ? currentRoom.markAllMessagesAsRead() : {}

                    RoundButton {
                        id: goTopFab
                        width: height
                        height: 64
                        visible: !parent.atYEnd

                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        contentItem: MaterialIcon {
                            anchors.fill: parent
                            icon: "\ue313"
                            color: "white"
                        }

                        opacity: pressed ? 1 : hovered ? 0.7 : 0.4
                        Material.background: Qt.lighter(Material.accent)

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

            Pane {
                padding: 16

                Layout.fillWidth: true
                Layout.preferredHeight: 80

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    ItemDelegate {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        contentItem: MaterialIcon { icon: "\ue226" }

                        onClicked: fileDialog.visible = true

                        FileDialog {
                            id: fileDialog
                            title: "Please choose a file"
                            folder: shortcuts.home
                            selectMultiple: false
                            onAccepted: {
                                currentRoom.uploadFile(fileUrl, fileUrl, matriqueController.getMIME(fileUrl))
                                var fileTransferProgressCallback = function(id, sent, total) {
                                    if (id == fileUrl) { inputField.progress = sent / total }
                                }
                                var completedCallback = function(id, localFile, mxcUrl) {
                                    if (id == fileUrl) {
                                        matriqueController.postFile(currentRoom, localFile, mxcUrl)
                                        inputField.progress = 0
                                        currentRoom.fileTransferCompleted.disconnect(fileTransferProgressCallback)
                                        currentRoom.fileTransferCompleted.disconnect(completedCallback)
                                    }
                                }

                                currentRoom.fileTransferProgress.connect(fileTransferProgressCallback)
                                currentRoom.fileTransferCompleted.connect(completedCallback)
                            }
                        }
                    }

                    TextField {
                        property real progress: 0

                        id: inputField
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        placeholderText: "Send a Message"
                        leftPadding: 16
                        topPadding: 0
                        bottomPadding: 0
                        selectByMouse: true

                        Keys.onReturnPressed: {
                            if (inputField.text) {
                                inputField.postMessage(inputField.text)
                                inputField.text = ""
                            }
                        }

                        background: Item {
                            Rectangle {
                                z: 5
                                width: inputField.width * inputField.progress
                                height: parent.height
                                color: Material.accent
                                opacity: 0.4
                            }
                            Rectangle { anchors.fill: parent; color: Material.theme == Material.Light ? "#eaeaea" : "#242424" }
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
                                var rainbowColor = ["#ee0000", "#ff7700", "#eeee00", "#00bb00", "#0000ee", "#dd00dd", "#880088"]
                                for (var i = 0; i < text.length; i++) {
                                    parsedText = parsedText + "<font color='" + rainbowColor[i % 7] + "'>" + text.charAt(i) + "</font>"
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
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        contentItem: MaterialIcon { icon: "\ue24e" }

                        background: Rectangle { color: Material.theme == Material.Light ? "#eaeaea" : "#242424" }
                    }
                }
            }
        }
    }
}
