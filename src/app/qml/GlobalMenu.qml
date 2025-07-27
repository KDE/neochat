// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import Qt.labs.platform as Labs

import QtQuick
import QtQuick.Window
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.settings

Labs.MenuBar {
    id: root

    required property NeoChatConnection connection

    Labs.Menu {
        title: i18nc("menu", "File")

        Labs.MenuItem {
            icon.name: "list-add-user"
            text: i18nc("@action:inmenu", "Find your Friends")
            enabled: pageStack.layers.currentItem.title !== i18n("Find your friends") && AccountRegistry.accountCount > 0
            onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Find your friends")
            })
        }
        Labs.MenuItem {
            icon.name: "system-users-symbolic"
            text: i18nc("@action:inmenu", "Create a Room…")
            enabled: pageStack.layers.currentItem.title !== i18n("Find your friends") && AccountRegistry.accountCount > 0
            shortcut: StandardKey.New
            onTriggered: {
                pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'CreateRoomDialog'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Create a Room")
                });
            }
        }
        Labs.MenuItem {
            icon.name: "compass-symbolic"
            text: i18nc("@action:inmenu", "Explore Rooms")
            onTriggered: {
                let dialog = pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
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
            enabled: pageStack.layers.currentItem.title !== i18n("Configure NeoChat…")
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
    EditMenu {
        title: i18nc("menu", "Edit")
        field: (root.activeFocusItem instanceof TextEdit || root.activeFocusItem instanceof TextInput) ? root.activeFocusItem : null
    }
    Labs.Menu {
        title: i18nc("menu", "View")

        Labs.MenuItem {
            icon.name: "search-symbolic"
            text: i18nc("@action:inmenu opens a UI element called the 'Quick Switcher', which offers a fast keyboard-based interface for switching in between chats.", "Search Rooms")
            onTriggered: quickSwitcher.open()
        }
    }
    Labs.Menu {
        title: i18nc("menu", "Window")

        Labs.MenuItem {
            icon.name: "view-fullscreen-symbolic"
            text: root.visibility === Window.FullScreen ? i18nc("menu", "Exit Full Screen") : i18nc("menu", "Enter Full Screen")
            onTriggered: root.visibility === Window.FullScreen ? root.showNormal() : root.showFullScreen()
        }
    }
    Labs.Menu {
        title: i18nc("menu", "Help")

        Labs.MenuItem {
            icon.name: "help-about-symbolic"
            text: i18nc("menu", "About NeoChat")
            onTriggered: pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"))
        }
        Labs.MenuItem {
            icon.name: "kde-symbolic"
            text: i18nc("menu", "About KDE")
            onTriggered: pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage"))
        }
    }
}
