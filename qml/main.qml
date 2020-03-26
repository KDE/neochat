import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.4 as Kirigami

import Spectral 0.1
import Spectral.Component 2.0
import Spectral.Panel 2.0

Kirigami.ApplicationWindow {
    id: root

    globalDrawer: Kirigami.GlobalDrawer {
        title: "Hello App"
        titleIcon: "applications-graphics"
        actions: [
            Kirigami.Action {
                text: "View"
                iconName: "view-list-icons"
                Kirigami.Action {
                    text: "action 1"
                }
                Kirigami.Action {
                    text: "action 2"
                }
                Kirigami.Action {
                    text: "action 3"
                }
            },
            Kirigami.Action {
                text: "action 3"
            }
        ]
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: roomListPanelComponent

    Controller {
        id: spectralController

        quitOnLastWindowClosed: true

        onErrorOccured: showPassiveNotification(error + ": " + detail)
    }

    RoomListModel {
        id: spectralRoomListModel

        connection: spectralController.connection
    }

    Binding {
        target: imageProvider
        property: "connection"
        value: spectralController.connection
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
