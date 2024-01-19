// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import Qt.labs.platform as Labs

import QtQuick
import QtQuick.Window
import QtQuick.Layouts

import org.kde.neochat
import org.kde.neochat.config
import org.kde.neochat.accounts

Labs.MenuBar {
    id: root

    required property NeoChatConnection connection

    Labs.Menu {
        title: i18nc("menu", "NeoChat")

        // TODO: make about page its own thing so we can go to it instead of settings where it's currently at
        // Labs.MenuItem {
        //     text: i18nc("menu", "About NeoChat")
        // }
        Labs.MenuItem {
            enabled: pageStack.layers.currentItem.title !== i18n("Configure NeoChat...")
            text: i18nc("menu", "Configure NeoChat...")

            shortcut: StandardKey.Preferences
            onTriggered: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/SettingsPage.qml", {
                connection: root.connection
            }, {
                title: i18n("Configure")
            })
        }
        Labs.MenuItem {
            text: i18nc("menu", "Quit NeoChat")

            shortcut: StandardKey.Quit
            onTriggered: Qt.quit()
        }
    }
    Labs.Menu {
        title: i18nc("menu", "File")

        Labs.MenuItem {
            text: i18nc("menu", "New Private Chat…")
            enabled: pageStack.layers.currentItem.title !== i18n("Start a Chat") && AccountRegistry.accountCount > 0
            onTriggered: pushReplaceLayer("qrc:/org/kde/neochat/qml/StartChatPage.qml", {connection: root.connection})
        }
        Labs.MenuItem {
            text: i18nc("menu", "New Group…")
            enabled: pageStack.layers.currentItem.title !== i18n("Start a Chat") && AccountRegistry.accountCount > 0
            shortcut: StandardKey.New
            onTriggered: {
                const dialog = createRoomDialog.createObject(root.overlay)
                dialog.open()
            }
        }
        Labs.MenuItem {
            text: i18nc("menu", "Browse Chats…")
            onTriggered: {
                let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/ExploreRoomsPage.qml", {connection: root.connection}, {title: i18nc("@title", "Explore Rooms")})
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    if (isJoined) {
                        RoomManager.enterRoom(root.connection.room(roomId))
                    } else {
                        RoomManager.resolveResource(roomId, "join")
                    }
                })
            }
        }
    }
    EditMenu {
        title: i18nc("menu", "Edit")
        field: (root.activeFocusItem instanceof TextEdit || root.activeFocusItem instanceof TextInput) ? root.activeFocusItem : null
    }
    Labs.Menu {
        title: i18nc("menu", "View")

        Labs.MenuItem {
            text: i18nc("menu item that opens a UI element called the 'Quick Switcher', which offers a fast keyboard-based interface for switching in between chats.", "Open Quick Switcher")
            shortcut: "Ctrl+K"
            onTriggered: quickView.item.open()
        }
    }
    Labs.Menu {
        title: i18nc("menu", "Window")

        // Labs.MenuItem {
        //     text: settings.userWantsSidebars ? i18nc("menu", "Hide Sidebar") : i18nc("menu", "Show Sidebar")
        //     onTriggered: settings.userWantsSidebars = !settings.userWantsSidebars
        // }
        Labs.MenuItem {
            text: root.visibility === Window.FullScreen ? i18nc("menu", "Exit Full Screen") : i18nc("menu", "Enter Full Screen")
            onTriggered: root.visibility === Window.FullScreen ? root.showNormal() : root.showFullScreen()
        }
    }
    // TODO: offline help system (https://invent.kde.org/network/neochat/-/issues/411)
    Labs.Menu {
        title: i18nc("menu", "Help")

        Labs.MenuItem {
            text: i18nc("menu", "Matrix FAQ")
            onTriggered: UrlHelper.openUrl("https://matrix.org/faq/")
        }
    }
}
