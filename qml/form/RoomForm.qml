import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3
import QtGraphicalEffects 1.0
import Matrique 0.1
import "qrc:/qml/component"

Item {
    id: item
    property var currentRoom

    Pane {
        anchors.fill: parent
        padding: 0

        background: Item {
            anchors.fill: parent
            visible: currentRoom === null
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

            visible: currentRoom !== null

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
                        source: currentRoom != null && currentRoom.avatarUrl != "" ? "image://mxc/" + currentRoom.avatarUrl : "qrc:/asset/img/avatar.png"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Label {
                            Layout.fillWidth: true
                            text: currentRoom !== null ? currentRoom.displayName : ""
                            font.pointSize: 16
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: currentRoom !== null ? currentRoom.topic : ""
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
                spacing: 12
                model: MessageEventModel{
                    id: messageEventModel
                    room: currentRoom
                }
                delegate: Row {
                    readonly property bool sentByMe: author === currentRoom.localUser

                    id: messageRow

                    anchors.right: sentByMe ? parent.right : undefined
                    spacing: 6

                    Image {
                        id: avatar
                        width: height
                        height: 40
                        mipmap: true
                        visible: !sentByMe
                        source: author.avatarUrl != "" ? "image://mxc/" + author.avatarUrl : "qrc:/asset/img/avatar.png"
                    }

                    Rectangle {
                        width: Math.min(messageText.implicitWidth + 24,
                                                           messageListView.width - (!sentByMe ? avatar.width + messageRow.spacing : 0))
                        height: messageText.implicitHeight + 24
                        color: sentByMe ? "lightgrey" : Material.accent

                        Label {
                            id: messageText
                            text: display
                            color: sentByMe ? "black" : "white"
                            anchors.fill: parent
                            anchors.margins: 12
                            wrapMode: Label.Wrap
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { /*anchors.left: messageListView.right*/ }
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
                            currentRoom.postMessage("text", inputField.text)
                            inputField.text = ""
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
