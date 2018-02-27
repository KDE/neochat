import QtQuick 2.10
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3
import Qt.labs.settings 1.0
import "qrc:/qml/component"

Page {
    property var window
    property var controller

    property alias homeserver: settings.server
    property alias username: settings.user
    property alias password: settings.pass

    Settings {
        id: settings

        property alias server: serverField.text
        property alias user: usernameField.text
        property alias pass: passwordField.text
    }

    Row {
        anchors.fill: parent

        Pane {
            width: parent.width / 2
            height: parent.height

            background: Item {
                Image {
                    id: background
                    anchors.fill: parent
                    source: "qrc:/asset/img/background.jpg"
                    fillMode: Image.PreserveAspectCrop
                }

                ColorOverlay {
                    anchors.fill: background
                    source: background
                    color: "#b000796b"
                }
            }

            Column {
                x: 32
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: "MATRIX LOGIN."
                    font.pointSize: 36
                    font.bold: true
                    color: "white"
                }

                Label {
                    text: "A NEW METHOD OF MESSAGING"
                    font.pointSize: 12
                    color: "white"
                }
            }
        }

        Pane {
            width: parent.width / 2
            height: parent.height
            padding: 64

            Column {
                id: main_col
                spacing: 8
                anchors.fill: parent

                ImageStatus {
                    width: 96
                    height: width
                    source: "qrc:/asset/img/avatar.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                RowLayout {
                    width: parent.width
                    height: 48
                    spacing: 0

                    Text {
                        text: "@"
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: parent.width * 0.05
                    }

                    TextField {
                        id: usernameField
                        Layout.preferredWidth: parent.width * 0.45
                        Layout.fillHeight: true
                        placeholderText: "Username"
                        leftPadding: 16
                        topPadding: 0
                        bottomPadding: 0

                        background: Rectangle {
                            color: "#eaeaea"
                            border.color: parent.activeFocus ? Material.accent : "transparent"
                            border.width: 2
                        }
                    }

                    Text {
                        text: ":"
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: parent.width * 0.05
                    }

                    TextField {
                        id: serverField
                        Layout.preferredWidth: parent.width * 0.45
                        Layout.fillHeight: true
                        placeholderText: "Server"
                        leftPadding: 16
                        topPadding: 0
                        bottomPadding: 0

                        background: Rectangle {
                            color: "#eaeaea"
                            border.color: parent.activeFocus ? Material.accent : "transparent"
                            border.width: 2
                        }
                    }
                }

                TextField {
                    id: passwordField
                    width: parent.width
                    height: 48
                    placeholderText: "Password"
                    leftPadding: 16
                    topPadding: 0
                    bottomPadding: 0

                    background: Rectangle {
                        color: "#eaeaea"
                        border.color: parent.activeFocus ? Material.accent : "transparent"
                        border.width: 2
                    }
                }

                Button {
                    id: loginButton
                    text: "LOGIN"
                    highlighted: true
                    width: parent.width

                    onClicked: controller.login(homeserver, username, password)
                }

                Button {
                    id: logoutButton
                    text: "LOGOUT"
                    flat: true
                    width: parent.width

                    onClicked: controller.logout()
                }
            }
        }
    }
}
