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

    Controller {
        id: controller

        onIsLoginChanged: console.log("Status:", isLogin)
    }

    Settings {
        id: settings

        property alias userID: controller.userID
        property alias token: controller.token
    }

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

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
            controller: controller
        }

        Contact {

        }

        Setting {

        }
    }
}
