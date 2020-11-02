import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import Spectral 0.1
import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Panel 2.0

Kirigami.ApplicationWindow {
    id: root
    property var currentRoom: null

    contextDrawer: RoomDrawer {
        id: contextDrawer
        enabled: roomList.enteredRoom !== null
        room: root.currentRoom
    }

    pageStack.initialPage: LoadingPage {}

    Component {
        id: roomListComponent
        RoomListPanel {
            id: roomList
            roomListModel: spectralRoomListModel

            onEnterRoom: {
                applicationWindow().pageStack.push(roomPanelComponent, {"currentRoom": room});
                root.currentRoom = room;

            }
            onLeaveRoom: {
                var stack = applicationWindow().pageStack;
                roomList.enteredRoom = null;

                stack.removePage(stack.lastItem);
            }
        }
    }

    Controller {
        id: spectralController

        quitOnLastWindowClosed: true

        onErrorOccured: showPassiveNotification(error + ": " + detail)

        onInitiated: {
            if (spectralController.accountCount === 0) {
                pageStack.replace("qrc:/qml/LoginPage.qml", {
                    'spectralController': spectralController
                });
            } else {
                pageStack.replace(roomListComponent);
            }
        }

        onConnectionAdded: {
            if (spectralController.accountCount === 1) {
                console.log("roomListComponent")
                pageStack.replace(roomListComponent);
            }
        }
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
