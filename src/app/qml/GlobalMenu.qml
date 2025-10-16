// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import Qt.labs.platform as Labs

import QtQuick
import QtQuick.Window

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.settings

Labs.MenuBar {
    id: root

    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow appWindow

    Labs.Menu {
        title: i18nc("menu", "File")

        Labs.MenuItem {
            icon.name: "list-add-user"
            text: i18nc("@action:inmenu", "Find User")
            enabled: root.connection
            onTriggered: root.appWindow.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Find User")
            })
        }
        Labs.MenuItem {
            icon.name: "system-users-symbolic"
            text: i18nc("@action:inmenu", "Create a Room…")
            enabled: root.connection
            shortcut: StandardKey.New
            onTriggered: {
                Qt.createComponent('org.kde.neochat', 'CreateRoomDialog').createObject(root.appWindow, {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Create a Room")
                }).open();
            }
        }
        Labs.MenuItem {
            icon.name: "compass-symbolic"
            text: i18nc("@action:inmenu", "Explore Rooms")
            enabled: root.connection
            onTriggered: {
                let dialog = root.appWindow.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Explore Rooms")
                });
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
                });
            }
        }
        Labs.MenuItem {
            text: i18nc("menu", "Configure NeoChat…")

            shortcut: StandardKey.Preferences
            onTriggered: NeoChatSettingsView.open()
        }
        Labs.MenuItem {
            text: i18nc("menu", "Quit NeoChat")

            shortcut: StandardKey.Quit
            onTriggered: Qt.quit()
        }
    }

    Labs.Menu {
        title: i18nc("menu", "View")

        Labs.MenuItem {
            icon.name: "search-symbolic"
            enabled: root.connection
            text: i18nc("@action:inmenu opens a UI element called the 'Quick Switcher', which offers a fast keyboard-based interface for switching in between chats.", "Search Rooms")
            onTriggered: (root.appWindow as Main).quickSwitcher.open()
        }
    }
    Labs.Menu {
        title: i18nc("menu", "Window")

        Labs.MenuItem {
            icon.name: "view-fullscreen-symbolic"
            text: root.appWindow.visibility === Window.FullScreen ? i18nc("menu", "Exit Full Screen") : i18nc("menu", "Enter Full Screen")
            onTriggered: root.appWindow.visibility === Window.FullScreen ? root.appWindow.showNormal() : root.appWindow.showFullScreen()
        }
    }
    Labs.Menu {
        title: i18nc("menu", "Help")

        Labs.MenuItem {
            icon.name: "help-about-symbolic"
            text: i18nc("menu", "About NeoChat")
            onTriggered: root.appWindow.pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"))
        }
        Labs.MenuItem {
            icon.name: "kde-symbolic"
            text: i18nc("menu", "About KDE")
            onTriggered: root.appWindow.pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage"))
        }
    }
}
