import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0 as Platform

import Spectral.Component 2.0
import Spectral.Page 2.0

import Spectral 0.1
import Spectral.Setting 0.1

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

    Platform.SystemTrayIcon {
        visible: MSettings.showTray
        iconSource: "qrc:/assets/img/icon.png"

        menu: Platform.Menu {
            Platform.MenuItem {
                text: qsTr("Hide Window")
                onTriggered: hideWindow()
            }
            Platform.MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }

        onActivated: showWindow()
    }

    Controller {
        id: spectralController

        quitOnLastWindowClosed: !MSettings.showTray

        onNotificationClicked: {
            roomPage.enteredRoom = currentConnection.room(roomId)
            roomPage.goToEvent(eventId)
            showWindow()
        }
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

                AutoListView {
                    property var currentConnection: null

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: accountListView

                    model: accountListModel

                    spacing: 0

                    clip: true

                    delegate: Column {
                        property bool expanded: accountListView.currentConnection === connection

                        width: parent.width

                        spacing: 0

                        SideNavButton {
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

                            onClicked: {
                                accountListView.currentConnection = connection
                                roomPage.filter = 0
                            }
                        }

                        Column {
                            width: parent.width
                            height: expanded ? implicitHeight : 0

                            spacing: 0
                            clip: true

                            SideNavButton {
                                width: parent.width
                                height: width

                                MaterialIcon {
                                    anchors.fill: parent

                                    icon: "\ue7f7"
                                    color:  "white"
                                }

                                onClicked: roomPage.filter = 1
                            }

                            SideNavButton {
                                width: parent.width
                                height: width

                                MaterialIcon {
                                    anchors.fill: parent

                                    icon: "\ue7fd"
                                    color:  "white"
                                }

                                onClicked: roomPage.filter = 2
                            }

                            SideNavButton {
                                width: parent.width
                                height: width

                                MaterialIcon {
                                    anchors.fill: parent

                                    icon: "\ue7fb"
                                    color:  "white"
                                }

                                onClicked: roomPage.filter = 3
                            }

                            Behavior on height {
                                PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
                            }
                        }
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

                    enabled: !addRoomMenu.opened
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
                                    AutoTextField {
                                        width: parent.width

                                        id: addRoomDialogNameTextField

                                        placeholderText: "Name"
                                    }
                                    AutoTextField {
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

                                contentItem: AutoTextField {
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

                                contentItem: AutoTextField {
                                    id: directChatDialogTextField
                                    placeholderText: "@bot:matrix.org"
                                }

                                onAccepted: spectralController.createDirectChat(currentConnection, directChatDialogTextField.text)
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

                    onClicked: MSettings.confirmOnExit ? confirmExitDialog.open() : Qt.quit()

                    Dialog {
                        x: (window.width - width) / 2
                        y: (window.height - height) / 2
                        width: 360

                        id: confirmExitDialog

                        parent: ApplicationWindow.overlay

                        title: "Exit"
                        modal: true
                        standardButtons: Dialog.Ok | Dialog.Cancel

                        contentItem: Column {
                            Label { text: "Exit?" }
                            CheckBox {
                                text: "Do not ask next time"
                                checked: !MSettings.confirmOnExit

                                onCheckedChanged: MSettings.confirmOnExit = !checked
                            }
                        }

                        onAccepted: Qt.quit()
                    }
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

    function showWindow() {
        window.show()
        window.raise()
        window.requestActivate()
    }

    function hideWindow() {
        window.hide()
    }

    Component.onCompleted: {
        spectralController.initiated.connect(function() {
            if (spectralController.accountCount == 0) stackView.push(loginPage)
        })
    }
}
