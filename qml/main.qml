import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.12 as Kirigami

import Spectral 0.1
import Spectral.Component 2.0
import Spectral.Panel 2.0

Kirigami.ApplicationWindow {
    id: root

    globalDrawer: SpectralSidebar { }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: roomListPanelComponent

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

    Component {
        id: roomListPanelComponent

        RoomListPanel {
            roomListModel: spectralRoomListModel

            onEnterRoom: {
                applicationWindow().pageStack.push(roomPanelComponent, {"currentRoom": room})
            }
            onLeaveRoom: {
                var stack = applicationWindow().pageStack;

                stack.removePage(stack.lastItem)
            }
        }
    }
}
