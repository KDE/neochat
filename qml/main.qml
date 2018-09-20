import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0
import Spectral 0.1
import Spectral.Settings 0.1

import "component"
import "form"
import "qrc:/js/util.js" as Util

ApplicationWindow {
    readonly property var currentConnection: accountListView.currentConnection ? accountListView.currentConnection : null

    width: 960
    height: 640
    minimumWidth: 720
    minimumHeight: 360

    id: window

    visible: true
    title: qsTr("Spectral")

    Material.theme: MSettings.darkTheme ? Material.Dark : Material.Light

    Material.accent: spectralController.color(currentConnection ? currentConnection.localUserId : "")

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

    Controller {
        id: spectralController

        onShowWindow: {
            window.show()
            window.raise()
            window.requestActivate()
        }
        onHideWindow: window.hide()
        onErrorOccured: {
            errorDialog.error = error
            errorDialog.detail = detail
            errorDialog.open()
        }
    }

    AccountListModel {
        id: accountListModel
        controller: spectralController
    }

    Dialog {
        property string error
        property string detail

        x: (window.width - width) / 2
        y: (window.height - height) / 2

        id: errorDialog

        title: error + " Error"
        contentItem: Label { text: errorDialog.detail }
    }

    Component {
        id: loginPage

        Login { controller: spectralController }
    }

    Room {
        id: roomPage

        parent: null

        connection: currentConnection
    }

    Setting {
        id: settingPage

        parent: null

        listModel: accountListModel
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 64
            Layout.fillHeight: true

            id: sideNav

            color: Material.primary

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                ListView {
                    property var currentConnection: null

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: accountListView

                    model: accountListModel

                    spacing: 0

                    clip: true

                    delegate: SideNavButton {
                        width: parent.width
                        height: width

                        selected: stackView.currentItem === page && currentConnection === connection

                        ImageItem {
                            anchors.fill: parent
                            anchors.margins: 12

                            hint: user.displayName
                            image: user.avatar
                        }

                        highlightColor: spectralController.color(user.id)

                        page: roomPage

                        onClicked: accountListView.currentConnection = connection
                    }
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
                                        width: parent.width

                                        id: addRoomDialogNameTextField

                                        placeholderText: "Name"
                                    }
                                    TextField {
                                        width: parent.width

                                        id: addRoomDialogTopicTextField

                                        placeholderText: "Topic"
                                    }
                                }

                                onAccepted: spectralController.createRoom(currentConnection, addRoomDialogNameTextField.text, addRoomDialogTopicTextField.text)
                            }
                        }
                        MenuItem {
                            text: "Join Room"

                            onTriggered: joinRoomDialog.open()

                            Dialog {
                                x: (window.width - width) / 2
                                y: (window.height - height) / 2
                                width: 360

                                id: joinRoomDialog

                                parent: ApplicationWindow.overlay

                                title: "Input Room Alias or ID"
                                modal: true
                                standardButtons: Dialog.Ok | Dialog.Cancel

                                contentItem: TextField {
                                    id: joinRoomDialogTextField
                                    placeholderText: "#matrix:matrix.org"
                                }

                                onAccepted: spectralController.joinRoom(currentConnection, joinRoomDialogTextField.text)
                            }
                        }

                        MenuItem {
                            text: "Direct Chat"

                            onTriggered: directChatDialog.open()

                            Dialog {
                                x: (window.width - width) / 2
                                y: (window.height - height) / 2
                                width: 360

                                id: directChatDialog

                                parent: ApplicationWindow.overlay

                                title: "Input User ID"
                                modal: true
                                standardButtons: Dialog.Ok | Dialog.Cancel

                                contentItem: TextField {
                                    id: directChatDialogTextField
                                    placeholderText: "@bot:matrix.org"
                                }

                                onAccepted: currentConnection.createDirectChat(directChatDialogTextField.text)
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
                        color: "white"
                    }
                    page: settingPage
                }

                SideNavButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width

                    MaterialIcon {
                        anchors.fill: parent

                        icon: "\ue8ac"
                        color: "white"
                    }

                    onClicked: Qt.quit()
                }
            }
        }

        StackView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: stackView

            initialItem: roomPage
        }
    }

    Binding {
        target: imageProvider
        property: "connection"
        value: currentConnection
    }

    Component.onCompleted: {
        spectralController.initiated.connect(function() {
            if (spectralController.accountCount == 0) stackView.push(loginPage)
        })
    }
}
