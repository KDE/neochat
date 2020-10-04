import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import Spectral 0.1
import Spectral.Component 2.0
import Spectral.Panel 2.0

Kirigami.ApplicationWindow {
    id: root
    property var currentRoom: null

    contextDrawer: RoomDrawer {
        id: contextDrawer
        enabled: roomList.enteredRoom !== null
        visible: enabled
        room: root.currentRoom
    }

    pageStack.initialPage: RoomListPanel {
        id: roomList
        roomListModel: spectralRoomListModel

        Component.onCompleted: {
            applicationWindow().pageStack.push(roomPanelComponent, {"currentRoom": roomList.enteredRoom })
        }

        onEnterRoom: {
            applicationWindow().pageStack.push(roomPanelComponent, {"currentRoom": room})
            root.currentRoom = room

        }
        onLeaveRoom: {
            var stack = applicationWindow().pageStack;
            roomList.enteredRoom = null

            stack.removePage(stack.lastItem)
        }
    }

    Controller {
        id: spectralController

        quitOnLastWindowClosed: true

        onErrorOccured: showPassiveNotification(error + ": " + detail)
    }

    Binding {
        target: imageProvider
        property: "connection"
        value: spectralController.connection
    }

    RoomListModel {
        id: spectralRoomListModel

        connection: spectralController.connection
    }

    Component {
        id: roomPanelComponent

        RoomPanel {
            currentRoom: root.currentRoom
        }
    }
}
