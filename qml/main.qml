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

    width: 960
    height: 640
    minimumWidth: 800
    minimumHeight: 480

    id: window

    visible: true
    title: qsTr("Matrique")

    Material.theme: MSettings.darkTheme ? Material.Dark : Material.Light

    FontLoader { id: materialFont; source: "qrc:/asset/font/material.ttf" }

    Controller {
        id: matriqueController

        onToggleWindow: {
            console.log("Activating window...")
            if (window.visible) {
                window.hide()
            } else {
                window.show()
                window.raise()
                window.requestActivate()
            }
        }
    }

    Popup {
        property bool busy: matriqueController.busy

        x: (window.width - width) / 2
        y: (window.height - height) / 2

        id: busyPopup

        modal: true
        focus: true

        closePolicy: Popup.NoAutoClose

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

        connection: accountListView.currentConnection
    }

    Setting {
        id: settingPage

        parent: null
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

                    model: AccountListModel { controller: matriqueController }

                    spacing: 0

                    delegate: SideNavButton {
                        width: parent.width
                        height: width

                        ImageItem {
                            anchors.fill: parent
                            anchors.margins: 12

                            hint: name
                            image: avatar
                            defaultColor: Material.accent
                        }

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

                                onAccepted: matriqueController.createRoom(addRoomDialogNameTextField.text, addRoomDialogTopicTextField.text)
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

                                onAccepted: matriqueController.joinRoom(joinRoomDialogTextField.text)
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
                        color: "white"
                    }
                    page: loginPage
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

                        icon: "\ue879"
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
        value: accountListView.currentConnection
    }
}
