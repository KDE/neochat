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
                contentItem: ImageStatus {
                    width: parent.width
                    height: parent.width
                    source: "qrc:/asset/img/avatar.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                page: Room {
                    id: roomPage
                    roomListModel: controller.roomListModel
                }
            }

            Rectangle {
                color: "transparent"
                Layout.fillHeight: true
            }

            ButtonDelegate {
                contentItem: Text {
                    text: "\ue853"
                    font.pointSize: 16
                    font.family: materialFont.name
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                page: Login {
                    id: loginPage
                    controller: controller
                }
            }

            ButtonDelegate {
                contentItem: Text {
                    text: "\ue8b8"
                    font.pointSize: 16
                    font.family: materialFont.name
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                page: Setting {

                }
            }

            ButtonDelegate {
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

    StackView {
        id: stackView
        anchors.fill: parent
        anchors.leftMargin: sideNav.width
        initialItem: roomPage
    }
}
