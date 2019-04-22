import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0 as Platform

import Spectral.Panel 2.0
import Spectral.Component 2.0
import Spectral.Page 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

ApplicationWindow {
    readonly property bool inPortrait: window.width < window.height

    Material.theme: MPalette.theme
    Material.background: MPalette.background

    width: 960
    height: 640
    minimumWidth: 480
    minimumHeight: 360

    id: window

    visible: true
    title: qsTr("Spectral")

    background: Rectangle {
        color: MSettings.darkTheme ? "#303030" : "#FFFFFF"
    }

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
            roomListForm.enteredRoom = spectralController.connection.room(roomId)
            roomForm.goToEvent(eventId)
            showWindow()
        }
        onErrorOccured: {
            roomListForm.errorControl.error = error
            roomListForm.errorControl.detail = detail
            roomListForm.errorControl.visible = true
        }
        onSyncDone: roomListForm.errorControl.visible = false
    }

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    Dialog {
        property bool busy: false

        width: 360
        x: (window.width - width) / 2
        y: (window.height - height) / 2

        id: loginDialog

        parent: ApplicationWindow.overlay

        title: "Login"

        contentItem: ColumnLayout {
            AutoTextField {
                Layout.fillWidth: true

                id: serverField

                placeholderText: "Server Address"
                text: "https://matrix.org"
            }

            AutoTextField {
                Layout.fillWidth: true

                id: usernameField

                placeholderText: "Username"

                onAccepted: passwordField.forceActiveFocus()
            }

            AutoTextField {
                Layout.fillWidth: true

                id: passwordField

                placeholderText: "Password"
                echoMode: TextInput.Password

                onAccepted: loginDialog.doLogin()
            }
        }

        footer: DialogButtonBox {
            Button {
                text: "Cancel"
                flat: true
                enabled: !loginDialog.busy

                onClicked: loginDialog.close()
            }

            Button {
                text: "OK"
                flat: true
                enabled: !loginDialog.busy

                onClicked: loginDialog.doLogin()
            }

            ToolTip {
                id: loginButtonTooltip

            }
        }

        onVisibleChanged: {
            if (visible) spectralController.onErrorOccured.connect(showError)
            else spectralController.onErrorOccured.disconnect(showError)
        }

        function showError(error, detail) {
            loginDialog.busy = false
            loginButtonTooltip.text = error + ": " + detail
            loginButtonTooltip.open()
        }

        function doLogin() {
            if (!(serverField.text.startsWith("http") && serverField.text.includes("://"))) {
                loginButtonTooltip.text = "Server address should start with http(s)://"
                loginButtonTooltip.open()
                return
            }

            loginDialog.busy = true
            spectralController.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)

            spectralController.connectionAdded.connect(function(conn) {
                busy = false
                loginDialog.close()
            })
        }
    }

    Dialog {
        anchors.centerIn: parent

        width: 480

        id: detailDialog

        contentItem: Column {
            id: detailColumn

            spacing: 0

            Repeater {
                model: AccountListModel{
                    controller: spectralController
                }

                delegate: Item {
                    width: detailColumn.width
                    height: 72

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12

                        spacing: 12

                        Avatar {
                            Layout.preferredWidth: height
                            Layout.fillHeight: true

                            source: user.avatarMediaId
                            hint: user.displayName || "No Name"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignHCenter

                            Label {
                                Layout.fillWidth: true

                                text: user.displayName || "No Name"
                                color: MPalette.foreground
                                font.pixelSize: 16
                                font.bold: true
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }

                            Label {
                                Layout.fillWidth: true

                                text: connection === spectralController.connection ? "Active" : "Online"
                                color: MPalette.lighter
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                        }
                    }

                    Menu {
                        id: contextMenu

                        MenuItem {
                            text: "Logout"

                            onClicked: spectralController.logout(connection)
                        }
                    }

                    RippleEffect {
                        anchors.fill: parent

                        onPrimaryClicked: spectralController.connection = connection
                        onSecondaryClicked: contextMenu.popup()
                    }
                }
            }

            RowLayout {
                width: parent.width

                MenuSeparator {
                    Layout.fillWidth: true
                }

                ToolButton {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    contentItem: MaterialIcon {
                        icon: "\ue145"
                        color: MPalette.lighter
                    }

                    onClicked: loginDialog.open()
                }
            }

            Control {
                width: parent.width

                contentItem: RowLayout {
                    MaterialIcon {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        color: MPalette.foreground
                        icon: "\ue7ff"
                    }

                    Label {
                        Layout.fillWidth: true

                        color: MPalette.foreground
                        text: "Start a Chat"
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: joinRoomDialog.open()
                }
            }

            Control {
                width: parent.width

                contentItem: RowLayout {
                    MaterialIcon {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        color: MPalette.foreground
                        icon: "\ue7fc"
                    }

                    Label {
                        Layout.fillWidth: true

                        color: MPalette.foreground
                        text: "Create a Room"
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: createRoomDialog.open()
                }
            }

            MenuSeparator {
                width: parent.width
            }

            Control {
                width: parent.width

                contentItem: RowLayout {
                    MaterialIcon {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        color: MPalette.foreground
                        icon: "\ue3a9"
                    }

                    Label {
                        Layout.fillWidth: true

                        color: MPalette.foreground
                        text: "Night Mode"
                    }

                    Switch {
                        id: darkThemeSwitch

                        checked: MSettings.darkTheme
                        onCheckedChanged: MSettings.darkTheme = checked
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: darkThemeSwitch.checked = !darkThemeSwitch.checked
                }
            }

            Control {
                width: parent.width

                contentItem: RowLayout {
                    MaterialIcon {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        color: MPalette.foreground
                        icon: "\ue5d2"
                    }

                    Label {
                        Layout.fillWidth: true

                        color: MPalette.foreground
                        text: "Enable System Tray"
                    }

                    Switch {
                        id: trayIconSwitch

                        checked: MSettings.showTray
                        onCheckedChanged: MSettings.showTray = checked
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: trayIconSwitch.checked = !trayIconSwitch.checked
                }
            }

            Control {
                width: parent.width

                contentItem: RowLayout {
                    MaterialIcon {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48

                        color: MPalette.foreground
                        icon: "\ue7f5"
                    }

                    Label {
                        Layout.fillWidth: true

                        color: MPalette.foreground
                        text: "Enable Notifications"
                    }

                    Switch {
                        id: notificationsSwitch

                        checked: MSettings.showNotification
                        onCheckedChanged: MSettings.showNotification = checked
                    }
                }

                RippleEffect {
                    anchors.fill: parent

                    onPrimaryClicked: notificationsSwitch.checked = !notificationsSwitch.checked
                }
            }
        }
    }

    Dialog {
        anchors.centerIn: parent
        width: 360

        id: joinRoomDialog

        title: "Start a Chat"

        contentItem: ColumnLayout {
            AutoTextField {
                Layout.fillWidth: true

                id: identifierField

                placeholderText: "Room Alias/User ID"
            }
        }

        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            var identifier = identifierField.text
            var firstChar = identifier.charAt(0)
            if (firstChar == "@") {
                spectralController.createDirectChat(spectralController.connection, identifier)
            } else if (firstChar == "!" || firstChar == "#") {
                spectralController.joinRoom(spectralController.connection, identifier)
            }
        }
    }

    Dialog {
        anchors.centerIn: parent
        width: 360

        id: createRoomDialog

        title: "Create a Room"

        contentItem: ColumnLayout {
            AutoTextField {
                Layout.fillWidth: true

                id: roomNameField

                placeholderText: "Room Name"
            }

            AutoTextField {
                Layout.fillWidth: true

                id: roomTopicField

                placeholderText: "Room Topic"
            }
        }

        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: spectralController.createRoom(spectralController.connection, roomNameField.text, roomTopicField.text)
    }

    Drawer {
        width: Math.min((inPortrait ? 0.67 : 0.3) * window.width, 360)
        height: window.height
        modal: inPortrait
        interactive: inPortrait
        position: inPortrait ? 0 : 1
        visible: !inPortrait

        id: roomListDrawer

        RoomListPanel {
            anchors.fill: parent

            id: roomListForm

            clip: true

            controller: spectralController

            onLeaveRoom: roomForm.saveReadMarker(room)
        }
    }

    RoomPanel {
        anchors.fill: parent
        anchors.leftMargin: !inPortrait ? roomListDrawer.width : undefined
        anchors.rightMargin: !inPortrait && roomDrawer.visible ? roomDrawer.width : undefined

        id: roomForm

        clip: true

        currentRoom: roomListForm.enteredRoom
    }

    RoomDrawer {
        width: Math.min((inPortrait ? 0.67 : 0.3) * window.width, 360)
        height: window.height
        modal: inPortrait
        interactive: inPortrait

        edge: Qt.RightEdge

        id: roomDrawer

        room: roomListForm.enteredRoom
    }

    Binding {
        target: imageProvider
        property: "connection"
        value: spectralController.connection
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
            if (spectralController.accountCount == 0) loginDialog.open()
        })
    }
}
