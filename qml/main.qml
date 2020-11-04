/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
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
        enabled: root.currentRoomm !== null
        room: root.currentRoom
        handleVisible: enabled && (pageStack.currentItem instanceof RoomPage)
    }

    globalDrawer: Kirigami.GlobalDrawer {
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("About Neochat")
                iconName: "help-about"
                onTriggered: pageStack.layers.push(aboutPage)
                enabled: pageStack.layers.currentItem.title !== i18n("About")
            }
        ]
    }

    Component {
        id: aboutPage
        Kirigami.AboutPage {
            aboutData: Controller.aboutData
        }
    }

    pageStack.initialPage: LoadingPage {}

    Component {
        id: roomListComponent
        RoomListPage {
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

    Connections {
        target: Controller

        onInitiated: {
            if (Controller.accountCount === 0) {
                pageStack.replace("qrc:/qml/LoginPage.qml", {});
            } else {
                pageStack.replace(roomListComponent);
            }
        }

        onConnectionAdded: {
            if (Controller.accountCount === 1) {
                console.log("roomListComponent")
                pageStack.replace(roomListComponent);
            }
        }
        onErrorOccured: showPassiveNotification(error + ": " + detail)
    }

    Binding {
        target: imageProvider
        property: "connection"
        value: Controller.connection
    }

    RoomListModel {
        id: spectralRoomListModel

        connection: Controller.connection
    }

    Component {
        id: roomPanelComponent

        RoomPage {
            currentRoom: root.currentRoom
        }
    }
}
