import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import Qt.labs.settings 1.0
import Spectral.Settings 0.1

import "component"

Page {
    property var controller

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
                    cache: false
                }

                ColorOverlay {
                    anchors.fill: background
                    source: background
                    color: Material.accent
                    opacity: 0.7
                }
            }

            Column {
                x: 32
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: "MATRIX LOGIN."
                    font.pointSize: 28
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

            ColumnLayout {
                width: parent.width

                id: mainCol

                TextField {
                    Layout.fillWidth: true

                    id: serverField

                    leftPadding: 16
                    topPadding: 0
                    bottomPadding: 0

                    text: "https://matrix.org"
                    placeholderText: "Server"

                    background: Rectangle {
                        implicitHeight: 48

                        color: MSettings.darkTheme ? "#242424" : "#eaeaea"
                        border.color: parent.activeFocus ? Material.accent : "transparent"
                        border.width: 2
                    }
                }

                TextField {
                    Layout.fillWidth: true

                    id: usernameField

                    leftPadding: 16
                    topPadding: 0
                    bottomPadding: 0

                    placeholderText: "Username"

                    background: Rectangle {
                        implicitHeight: 48

                        color: MSettings.darkTheme ? "#242424" : "#eaeaea"
                        border.color: parent.activeFocus ? Material.accent : "transparent"
                        border.width: 2
                    }
                }

                TextField {
                    Layout.fillWidth: true

                    id: passwordField

                    leftPadding: 16
                    topPadding: 0
                    bottomPadding: 0

                    placeholderText: "Password"
                    echoMode: TextInput.Password

                    background: Rectangle {
                        implicitHeight: 48

                        color: MSettings.darkTheme ? "#242424" : "#eaeaea"
                        border.color: parent.activeFocus ? Material.accent : "transparent"
                        border.width: 2
                    }
                }

                Button {
                    Layout.fillWidth: true

                    id: loginButton

                    text: "LOGIN"
                    highlighted: true

                    ToolTip {
                        id: loginButtonTooltip
                    }

                    onClicked: {
                        if (!(serverField.text.startsWith("http") && serverField.text.includes("://"))) {
                            loginButtonTooltip.text = "Server address should start with http(s)://"
                            loginButtonTooltip.open()
                            return
                        }
                        if (!(usernameField.text.startsWith("@") && usernameField.text.includes(":"))) {
                            loginButtonTooltip.text = "Username should be in format of @example:example.com"
                            loginButtonTooltip.open()
                            return
                        }

                        controller.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)

                        controller.connectionAdded.connect(function() { stackView.pop() })
                    }
                }
            }
        }
    }
}
