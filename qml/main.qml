import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import Qt.labs.settings 1.1
import Qt.labs.platform 1.1 as Platform

import Spectral.Panel 2.0
import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

ApplicationWindow {
    readonly property bool inPortrait: window.width < 640

    Material.theme: MPalette.theme
    Material.background: MPalette.background

    width: 960
    height: 640
    minimumWidth: 480
    minimumHeight: 360

    id: window

    visible: true
    title: qsTr("Spectral")

    font.family: MSettings.fontFamily

    background: Rectangle {
        color: MSettings.darkTheme ? "#303030" : "#FFFFFF"
    }

    TrayIcon {
        id: trayIcon

        visible: MSettings.showTray

        iconSource: ":/assets/img/icon.png"

        isOnline: spectralController.isOnline

        onShowWindow: window.showWindow()
    }

    Platform.MenuBar {
        id: menuBar

        Platform.Menu {
            id: fileMenu
            title: "File"

            Platform.MenuItem {
                text: "Preferences"

                shortcut: StandardKey.Preferences
                role: Platform.MenuItem.PreferencesRole

                onTriggered: accountDetailDialog.createObject(window).open()
            }

            Platform.MenuItem {
                text: "Quit"

                shortcut: StandardKey.Quit
                role: Platform.MenuItem.QuitRole

                onTriggered: Qt.quit()
            }
        }
    }

    Controller {
        id: spectralController

        quitOnLastWindowClosed: !MSettings.showTray

        onErrorOccured: errorControl.show(error + ": " + detail, 3000)
    }

    NotificationsManager {
        id: notificationsManager

        onNotificationClicked: {
            roomListForm.enteredRoom = spectralController.connection.room(roomId)
            roomForm.goToEvent(eventId)
            showWindow()
        }
    }

    Shortcut {
        sequence: "Ctrl+Q"
        context: Qt.ApplicationShortcut
        onActivated: Qt.quit()
    }

    ToolTip {
        id: busyIndicator

        parent: ApplicationWindow.overlay

        visible: spectralController.busy
        text: "Loading, please wait"

        font.pixelSize: 14
    }

    ToolTip {
        id: errorControl

        parent: ApplicationWindow.overlay

        font.pixelSize: 14
    }

    Component {
        id: accountDetailDialog

        AccountDetailDialog {}
    }

    Component {
        id: loginDialog

        LoginDialog {}
    }

    Component {
        id: joinRoomDialog

        JoinRoomDialog {}
    }

    Component {
        id: startChatDialog

        StartChatDialog {}
    }

    Component {
        id: createRoomDialog

        CreateRoomDialog {}
    }

    Component {
        id: fontFamilyDialog

        FontFamilyDialog {}
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

            connection: spectralController.connection

            onLeaveRoom: roomForm.saveViewport()
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
            if (spectralController.accountCount == 0) loginDialog.createObject(window).open()
        })
    }
}
