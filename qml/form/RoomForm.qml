import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Controls.Material 2.4
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
            Pane {
                anchors.fill: parent
            }

            Label {
                z: 10
                text: "Please choose a room."
                anchors.centerIn: parent
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            visible: currentRoom

            Pane {
                z: 10
                padding: 16

                Layout.fillWidth: true
                Layout.preferredHeight: 80

                background: Rectangle {
                    color: Material.theme == Material.Light ? "#eaeaea" : "#242424"
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 16

                    ImageStatus {
                        Layout.preferredWidth: parent.height
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

            ListView {
                id: messageListView

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                displayMarginBeginning: 40
                displayMarginEnd: 40
                verticalLayoutDirection: ListView.BottomToTop
                maximumFlickVelocity: 1024
                spacing: 12

                model: MessageEventModel{
                    id: messageEventModel
                    room: currentRoom

                    onRoomChanged: if (room.timelineSize === 0) room.getPreviousContent(50)
                }

                delegate: MessageDelegate {}

                onAtYBeginningChanged: if (atYBeginning && currentRoom) currentRoom.getPreviousContent(50)

                ScrollBar.vertical: ScrollBar {}

                Behavior on contentY {
                    PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
                }

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

                    Behavior on opacity {
                        PropertyAnimation { easing.type: Easing.Linear; duration: 200 }
                    }
                }
            }

            Pane {
                z: 10
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
                    }

                    TextField {
                        id: inputField
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        placeholderText: "Send a Message"
                        leftPadding: 16
                        topPadding: 0
                        bottomPadding: 0
                        selectByMouse: true

                        background: Rectangle {
                            color: Material.theme == Material.Light ? "#eaeaea" : "#242424"
                        }

                        Keys.onReturnPressed: {
                            postMessage(inputField.text)
                            inputField.text = ""
                        }

                        function postMessage(text) {
                            if (text.trim().length === 0) {
                                return
                            }
                            if(!currentRoom) {
                                return
                            }

                            var type = "m.text"
                            var PREFIX_ME = '/me '
                            if (text.indexOf(PREFIX_ME) === 0) {
                                text = text.substr(PREFIX_ME.length)
                                type = "m.emote"
                            }

                            //                            var parsedText = Markdown.markdown_parser(text)
                            currentRoom.postMessage(type, text)
                        }
                    }

                    ItemDelegate {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        contentItem: MaterialIcon { icon: "\ue24e" }

                        background: Rectangle {
                            color: Material.theme == Material.Light ? "#eaeaea" : "#242424"
                        }
                    }
                }
            }
        }
    }
}
