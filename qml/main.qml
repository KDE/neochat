import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0

import "qrc:/qml/component"
import "qrc:/qml/form"

import Matrique 0.1

ApplicationWindow {
    id: window
    visible: true
    width: 960
    height: 640
    title: qsTr("Matrique")

    Connection {
        id: connection
        homeserver: settings.homeserver
    }

    Settings {
        id: settings

        property string homeserver

        property string userID
        property string token
        property string deviceID
    }

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

    function init() {
        connection.connected.connect(function() {
            console.info("Matrix connected.")

            connection.syncError.connect(reconnect)
            connection.resolveError.connect(reconnect)
            connection.syncDone.connect(resync)
        })
    }

    function resync() {
        if(!initialised) {
        }
        connection.sync(30000)
    }

    function reconnect() {
        connection.connectWithToken(connection.localUserId,
                                    connection.accessToken,
                                    connection.deviceId)
    }

    function login() {
        if(!settings.homeserver) settings.homeserver = "https://matrix.org"

        console.info("Homeserver:", connection.homeserver)
        console.info("UserID:", settings.userID)
        console.info("Token:", settings.token)
        console.info("DeviceID:", settings.deviceID)

        if(!settings.token || !settings.userID) {
            console.info("Using server address.")
            settings.homeserver = loginPage.homeserver

            function saveCredentials() {
                settings.userID = connection.localUserId
                settings.token = connection.accessToken

                connection.connected.disconnect(saveCredentials)
            }

            connection.connected.connect(saveCredentials)

            connection.connectToServer(loginPage.username, loginPage.password, connection.deviceId)
        } else {
            console.info("Using token")
            connection.connectWithToken(settings.userID, settings.token, connection.deviceId)
        }
    }

    function logout() {
        settings.homeserver = null;
        settings.userID = null;
        settings.token = null;
    }

    SideNav {
        id: sideNav
        width: 80
        height: window.height

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            ButtonDelegate {
                index: 0

                contentItem: ImageStatus {
                    width: parent.width
                    height: parent.width
                    source: "qrc:/asset/img/avatar.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            Rectangle {
                color: "transparent"
                Layout.fillHeight: true
            }

            ButtonDelegate {
                index: 1

                contentItem: Text {
                    text: "\ue853"
                    font.pointSize: 16
                    font.family: materialFont.name
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            ButtonDelegate {
                index: 2

                contentItem: Text {
                    text: "\ue5d2"
                    font.pointSize: 16
                    font.family: materialFont.name
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            ButtonDelegate {
                index: 3

                contentItem: Text {
                    text: "\ue8b8"
                    font.pointSize: 16
                    font.family: materialFont.name
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            ButtonDelegate {
                index: 4

                contentItem: Text {
                    text: "\ue879"
                    font.pointSize: 16
                    font.family: materialFont.name
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: Qt.quit()
            }
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        anchors.leftMargin: sideNav.width
        interactive: false
        orientation: Qt.Vertical

        Home {

        }

        Login {
            id: loginPage
            window: window
        }

        Contact {

        }

        Setting {

        }
    }

    Component.onCompleted: {
        init()
    }
}
