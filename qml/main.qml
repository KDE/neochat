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
    property int columnWidth: Kirigami.Units.gridUnit * 13

    minimumWidth: Kirigami.Units.gridUnit * 15
    minimumHeight: Kirigami.Units.gridUnit * 20

    wideScreen: width > columnWidth * 5

    Connections {
        target: root.quitAction
        function onTriggered() {
            Qt.quit()
        }
    }

    /**
     * Manage opening and close rooms
     */
    QtObject {
        id: roomManager

        property var currentRoom: null
        property alias pageStack: root.pageStack
        property bool invitationOpen: false
        property var roomList: null
        property Item roomItem: null

        readonly property bool hasOpenRoom: currentRoom !== null

        signal leaveRoom(string room);
        signal openRoom(string room);

        function loadInitialRoom() {
            if (Config.openRoom) {
                const room = Controller.activeConnection.room(Config.openRoom);
                currentRoom = room;
                roomItem = pageStack.push(roomPage, { 'currentRoom': room, });
                connectRoomToSignal(roomItem);
            } else {
                // TODO create welcome page
            }
        }

        function enterRoom(room) {
            let item = null;
            if (currentRoom != null || invitationOpen) {
                roomItem.currentRoom = room;
                pageStack.currentIndex = pageStack.depth - 1;
            } else {
                roomItem = pageStack.push(roomPage, { 'currentRoom': room, });
            }
            currentRoom = room;
            Config.openRoom = room.id;
            Config.save();
            connectRoomToSignal(roomItem);
            return roomItem;
        }

        function openInvitation(room) {
            if (currentRoom != null) {
                currentRoom = null;
                pageStack.removePage(pageStack.lastItem);
            }
            invitationOpen = true;
            pageStack.push("qrc:/imports/NeoChat/Page/InvitationPage.qml", {"room": room});
        }

        function getBack() {
            pageStack.replace(roomPage, { 'currentRoom': currentRoom, });
        }

        function connectRoomToSignal(item) {
            if (!roomList) {
                console.log("Should not happen: no room list page but room page");
            }
            item.switchRoomUp.connect(function() {
                roomList.goToNextRoom();
            });

            item.switchRoomDown.connect(function() {
                roomList.goToPreviousRoom();
            });
        }
    }

    function pushReplaceLayer(page, args) {
        if (pageStack.layers.depth === 2) {
            pageStack.layers.replace(page, args);
        } else {
            pageStack.layers.push(page, args);
        }
    }

    function showWindow() {
        root.show()
        root.raise()
        root.requestActivate()
    }

    contextDrawer: RoomDrawer {
        id: contextDrawer
        contentItem.implicitWidth: columnWidth
        edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge
        modal: !root.wideScreen
        onEnabledChanged: drawerOpen = enabled && !modal
        onModalChanged: drawerOpen = !modal
        enabled: roomManager.hasOpenRoom && pageStack.layers.depth < 2 && pageStack.depth < 3
        room: roomManager.currentRoom
        handleVisible: enabled && pageStack.layers.depth < 2 && pageStack.depth < 3
    }

    globalDrawer: Kirigami.GlobalDrawer {
        property bool hasLayer
        contentItem.implicitWidth: columnWidth
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Explore rooms")
                icon.name: "compass"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/JoinRoomPage.qml", {"connection": Controller.activeConnection})
                enabled: pageStack.layers.currentItem.title !== i18n("Explore Rooms")
            },
            Kirigami.Action {
                text: i18n("Start a Chat")
                icon.name: "irc-join-channel"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/StartChatPage.qml", {"connection": Controller.activeConnection})
                enabled: pageStack.layers.currentItem.title !== i18n("Start a Chat")
            },
            Kirigami.Action {
                text: i18n("Create a Room")
                icon.name: "irc-join-channel"
                onTriggered: {
                    let dialog = createRoomDialog.createObject(root.overlay);
                    dialog.open();
                }
                shortcut: StandardKey.New
                enabled: pageStack.layers.currentItem.title !== i18n("Start a Chat")
            },
            Kirigami.Action {
                text: i18n("Accounts")
                icon.name: "im-user"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/AccountsPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Accounts")
            },
            Kirigami.Action {
                text: i18n("Devices")
                iconName: "network-connect"
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/DevicesPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Devices")
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "settings-configure"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/SettingsPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Settings")
                shortcut: StandardKey.Preferences
            },
            Kirigami.Action {
                text: i18n("About Neochat")
                icon.name: "help-about"
                onTriggered: pushReplaceLayer(aboutPage)
                enabled: pageStack.layers.currentItem.title !== i18n("About")
            },
            Kirigami.Action {
                text: i18n("Logout")
                icon.name: "list-remove-user"
                onTriggered: Controller.logout(Controller.activeConnection, true)
            },
            Kirigami.Action {
                text: i18n("Quit")
                icon.name: "gtk-quit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
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

        function onInitiated() {
            if (Controller.accountCount === 0) {
                pageStack.replace("qrc:/imports/NeoChat/Page/LoginPage.qml", {});
            } else {
                roomManager.roomList = pageStack.replace(roomListComponent, {'activeConnection': Controller.activeConnection});
                roomManager.loadInitialRoom();
            }
        }

        function onConnectionAdded() {
            if (Controller.accountCount === 1) {
                if (Controller.busy) {
                    pageStack.replace("qrc:/imports/NeoChat/Page/LoadingPage.qml");
                } else {
                    roomManager.roomList = pageStack.replace(roomListComponent);
                }
            }
        }

        function onBusyChanged() {
            if(!Controller.busy && roomManager.roomList === null) {
                roomManager.roomList = pageStack.replace(roomListComponent);
            }
        }

        function onConnectionDropped() {
            if (Controller.accountCount === 0) {
                pageStack.clear();
                pageStack.replace("qrc:/imports/NeoChat/Page/LoginPage.qml");
            }
        }

        function onGlobalErrorOccured(error, detail) {
            showPassiveNotification(error + ": " + detail)
        }

        function onShowWindow() {
            root.showWindow()
        }

        function onOpenRoom(room) {
            roomManager.enterRoom(room)
        }
    }

    RoomListModel {
        id: spectralRoomListModel

        connection: Controller.activeConnection
    }

    Component {
        id: roomPage

        RoomPage {}
    }

    Component {
        id: createRoomDialog

        CreateRoomDialog {}
    }
}
