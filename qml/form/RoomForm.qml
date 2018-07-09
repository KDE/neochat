import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Controls.Material 2.4
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
                spacing: 12

                model: MessageEventModel{
                    id: messageEventModel
                    room: currentRoom

                    onModelReset: currentRoom.getPreviousContent(50)
                }

                delegate: Row {
                    readonly property bool sentByMe: author === currentRoom.localUser

                    id: messageRow

                    anchors.right: sentByMe ? parent.right : undefined
                    spacing: 6

                    ImageStatus {
                        id: avatar
                        width: height
                        height: 40
                        round: false
                        visible: !sentByMe
                        source: author.avatarUrl != "" ? "image://mxc/" + author.avatarUrl : null
                        displayText: author.displayName

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent

                            ToolTip.visible: pressed
                            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                            ToolTip.text: author.displayName
                        }
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
                            linkColor: sentByMe ? Material.accent : "white"
                            anchors.fill: parent
                            anchors.margins: 12
                            wrapMode: Label.Wrap
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { /*anchors.left: messageListView.right*/ }

                Behavior on contentY {
                    PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
                }

                RoundButton {
                    id: goTopFab
                    width: height
                    height: !parent.atYEnd ? 64 : 0

                    anchors.verticalCenter: parent.bottom
                    anchors.verticalCenterOffset: -48
                    anchors.horizontalCenter: parent.right
                    anchors.horizontalCenterOffset: -48

                    contentItem: MaterialIcon {
                        anchors.fill: parent
                        icon: "\ue313"
                        color: "white"
                    }

                    opacity: hovered ? 1 : 0.5
                    Material.background: Qt.lighter(Material.accent)

                    onClicked: parent.positionViewAtBeginning()

                    Behavior on height {
                        PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
                    }
                    Behavior on opacity {
                        PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
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
