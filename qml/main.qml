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

    Connection { id: connection }

    Settings {
        id: settings

        property alias user: loginPage.username
        property alias pass: loginPage.password
        property var token
    }

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

    function login() {
        console.info("Login is invoked.")

        var connect = connection.connectToServer

        connection.connected.connect(function() {
            settings.user = connection.userId()
            settings.token = connection.accessToken

            connection.connectionError.connect(connection.reconnect)
            connection.syncDone.connect(resync)
            connection.reconnected.connect(resync)

            connection.sync()
        })

        var userParts = settings.user.split(':')
        if(userParts.length === 1 || userParts[1] === "matrix.org") { // If this user uses default server.
            console.info("Matrix server is used.")
            connect(settings.user, settings.pass, "Device")
        } else {
            connection.resolved.connect(function() {
                connect(settings.user, settings.pass, "Device")
            })
            connection.resolveError.connect(function() {
                console.info("Couldn't resolve server!")
            })
            connection.resolveServer(userParts[1])
        }
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
}
