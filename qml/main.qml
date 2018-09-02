import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0
import Matrique 0.1
import Matrique.Settings 0.1

import "component"
import "form"

ApplicationWindow {
    readonly property var connection: matriqueController.connection

    id: window
    visible: true
    width: 960
    height: 640
    minimumWidth: 800
    minimumHeight: 480
    title: qsTr("Matrique")

    Material.theme: MSettings.darkTheme ? Material.Dark : Material.Light

    Settings {
        property alias homeserver: matriqueController.homeserver
        property alias userID: matriqueController.userID
        property alias token: matriqueController.token
    }

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

    Controller {
        id: matriqueController

        onToggleWindow: window.visible ? window.hide() : window.show()
    }

    Popup {
        property bool busy: matriqueController.busy

        id: busyPopup

        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        focus: true

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        BusyIndicator { running: true }

        onBusyChanged: busyPopup.busy ? busyPopup.open() : busyPopup.close()
    }

    Component {
        id: loginPage

        Login { controller: matriqueController }
    }

    Room {
        id: roomPage

        parent: null

        connection: matriqueController.isLogin ? window.connection : null
    }

    Setting {
        id: settingPage

        parent: null

        connection: window.connection
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: sideNav
            Layout.preferredWidth: 64
            Layout.fillHeight: true
            color: Material.primary

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                SideNavButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width

                    ImageStatus {
                        anchors.fill: parent
                        anchors.margins: 12

                        source: matriqueController.isLogin ? connection.localUser && connection.localUser.avatarUrl ? "image://mxc/" + connection.localUser.avatarUrl : "" : "qrc:/asset/img/avatar.png"
                        displayText: matriqueController.isLogin && connection.localUser.displayName ? connection.localUser.displayName : ""
                    }

                    page: roomPage
                }

                Rectangle {
                    color: "transparent"
                    Layout.fillHeight: true
                }

                SideNavButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width

                    MaterialIcon {
                        anchors.fill: parent

                        icon: "\ue145"
                        color:  "white"
                    }

                    onClicked: addRoomMenu.popup()

                    Menu {
                        id: addRoomMenu
                        MenuItem {
                            text:"New Room"
                            onTriggered: addRoomDialog.open()

                            Dialog {
                                id: addRoomDialog
                                parent: ApplicationWindow.overlay

                                x: (window.width - width) / 2
                                y: (window.height - height) / 2
                                width: 360

                                title: "New Room"
                                modal: true
                                standardButtons: Dialog.Ok | Dialog.Cancel

                                contentItem: Column {
                                    TextField {
                                        id: addRoomDialogNameTextField
                                        width: parent.width

                                        placeholderText: "Name"
                                    }
                                    TextField {
                                        id: addRoomDialogTopicTextField
                                        width: parent.width
                                        placeholderText: "Topic"
                                    }
                                }

                                onAccepted: matriqueController.createRoom(addRoomDialogNameTextField.text, addRoomDialogTopicTextField.text)
                            }
                        }
                        MenuItem {
                            text: "Join Room"
                            onTriggered: joinRoomDialog.open()

                            Dialog {
                                id: joinRoomDialog
                                parent: ApplicationWindow.overlay

                                x: (window.width - width) / 2
                                y: (window.height - height) / 2
                                width: 360

                                title: "Input Room Alias or ID"
                                modal: true
                                standardButtons: Dialog.Ok | Dialog.Cancel

                                contentItem: TextField {
                                    id: joinRoomDialogTextField
                                    placeholderText: "#matrix:matrix.org"
                                }

                                onAccepted: matriqueController.joinRoom(joinRoomDialogTextField.text)
                            }
                        }

                        MenuItem {
                            text: "Direct Chat"
                            onTriggered: directChatDialog.open()

                            Dialog {
                                id: directChatDialog
                                parent: ApplicationWindow.overlay

                                x: (window.width - width) / 2
                                y: (window.height - height) / 2
                                width: 360

                                title: "Input User ID"
                                modal: true
                                standardButtons: Dialog.Ok | Dialog.Cancel

                                contentItem: TextField {
                                    id: directChatDialogTextField
                                    placeholderText: "@bot:matrix.org"
                                }

                                onAccepted: matriqueController.createDirectChat(directChatDialogTextField.text)
                            }
                        }
                    }
                }

                SideNavButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width

                    MaterialIcon {
                        anchors.fill: parent

                        icon: "\ue8b8"
                        color: parent.highlighted ? Material.accent : "white"
                    }
                    page: settingPage
                }

                SideNavButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width

                    MaterialIcon {
                        anchors.fill: parent

                        icon: "\ue879"
                        color: "white"
                    }

                    onClicked: Qt.quit()
                }
            }
        }

        StackView {
            id: stackView
            initialItem: roomPage

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Component.onCompleted: {
        imageProvider.connection = matriqueController.connection

        if (matriqueController.userID && matriqueController.token) {
            matriqueController.login();
        } else {
            stackView.replace(loginPage);
        }
    }
}
