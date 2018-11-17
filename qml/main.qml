import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0 as Platform

import Spectral.Panel 2.0
import Spectral.Component 2.0
import Spectral.Page 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

import "qrc:/js/util.js" as Util

ApplicationWindow {
    Material.theme: MSettings.darkTheme ? Material.Dark : Material.Light

    width: 960
    height: 640
    minimumWidth: 720
    minimumHeight: 360

    id: window

    visible: true
    title: qsTr("Spectral")

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
            roomPage.enteredRoom = spectralController.connection.room(roomId)
            roomPage.goToEvent(eventId)
            showWindow()
        }
        onErrorOccured: {
            roomListForm.errorControl.error = error
            roomListForm.errorControl.detail = detail
            roomListForm.errorControl.visible = true
        }
        onSyncDone: roomListForm.errorControl.visible = false
    }

    SplitView {
        anchors.fill: parent

        RoomListPanel {
            width: window.width * 0.35
            Layout.minimumWidth: 180

            id: roomListForm

            clip: true

            controller: spectralController

            onLeaveRoom: roomForm.saveReadMarker(room)
        }

        RoomPanel {
            Layout.fillWidth: true
            Layout.minimumWidth: 480

            id: roomForm

            clip: true

            currentRoom: roomListForm.enteredRoom
        }
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

    function showError() {

    }

//    Component.onCompleted: {
//        spectralController.initiated.connect(function() {
//            if (spectralController.accountCount == 0) stackView.push(loginPage)
//        })
//    }
}
