/**
 * SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.14
import QtQuick.Controls 2.14 as QQC2
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Panel 1.0
import NeoChat.Dialog 1.0
import NeoChat.Page 1.0

Kirigami.ApplicationWindow {
    id: root
    property var currentRoom: null

    /**
     * Manage opening and close rooms
     */
    QtObject {
        id: roomManager

        property var currentRoom: null
        property alias pageStack: root.pageStack

        readonly property bool hasOpenRoom: currentRoom !== null

        signal leaveRoom(string room);
        signal openRoom(string room);

        function loadInitialRoom() {
            if (Config.openRoom) {
                const room = Controller.activeConnection.room(Config.openRoom);
                currentRoom = room;
                pageStack.push(roomPage, { 'currentRoom': room, });
            } else {
                // TODO create welcome page
            }
        }


        function enterRoom(room) {
            if (currentRoom != null) {
                currentRoom = null;
                pageStack.removePage(pageStack.lastItem);
            }
            var item = pageStack.push(roomPage, { 'currentRoom': room, });
            currentRoom = room;
            Config.openRoom = room.id;
            Config.save();
            return item;
        }
    }

    contextDrawer: RoomDrawer {
        id: contextDrawer
        enabled: roomManager.hasOpenRoom
        room: roomManager.currentRoom
        handleVisible: enabled && (pageStack.currentItem instanceof RoomPage)
    }

    globalDrawer: Kirigami.GlobalDrawer {
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Explore rooms")
                iconName: "compass"
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/JoinRoomPage.qml", {"connection": Controller.activeConnection})
                enabled: pageStack.layers.currentItem.title !== i18n("Explore Rooms")
            },
            Kirigami.Action {
                text: i18n("Accounts")
                iconName: "im-user"
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/AccountsPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Accounts")
            },
            Kirigami.Action {
                text: i18n("Settings")
                iconName: "settings-configure"
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/SettingsPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Settings")
            },
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
            activeConnection: Controller.activeConnection
        }
    }

    Connections {
        target: Controller

        onInitiated: {
            if (Controller.accountCount === 0) {
                pageStack.replace("qrc:/imports/NeoChat/Page/LoginPage.qml", {});
            } else {
                pageStack.replace(roomListComponent, {'activeConnection': Controller.activeConnection});
                roomManager.loadInitialRoom();
            }
        }

        onConnectionAdded: {
            if (Controller.accountCount === 1) {
                pageStack.replace(roomListComponent);
            }
        }

        onConnectionDropped: {
            if(Controller.accountCount === 0)
                pageStack.replace("qrc:/imports/NeoChat/Page/LoginPage.qml")
        }

        onErrorOccured: showPassiveNotification(error + ": " + detail)
    }

    RoomListModel {
        id: spectralRoomListModel

        connection: Controller.activeConnection
    }

    Component {
        id: roomPage

        RoomPage {}
    }
}
