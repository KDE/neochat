// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat
import org.kde.neochat.timeline as Timeline
import org.kde.neochat.settings as Settings

/**
 * @brief Page for holding a room drawer component.
 *
 * This the companion component to RoomDrawer and is designed to be used on mobile
 * where we want the room drawer to be pushed as a page as thin drawer doesn't
 * look good.
 *
 * @sa RoomDrawer
 */
Kirigami.Page {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom room
    required property NeoChatConnection connection
    required property UserListModel userListModel
    required property Timeline.MediaMessageFilterModel mediaMessageFilterModel

    signal resolveResource(string idOrUri, string action)

    title: drawerItemLoader.item ? drawerItemLoader.item.title : ""

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component.onCompleted: infoAction.toggle()

    actions: [
        Kirigami.Action {
            displayHint: Kirigami.DisplayHint.IconOnly
            text: i18nc("@action:button", "Room settings")
            icon.name: 'settings-configure-symbolic'
            onTriggered: {
                Settings.RoomSettingsView.openRoomSettings(root.room, Settings.RoomSettingsView.Room);
            }
        }
    ]

    Loader {
        id: drawerItemLoader
        width: parent.width
        height: parent.height
        sourceComponent: roomInformation
    }

    Component {
        id: roomInformation
        RoomInformation {
            room: root.room
            userListModel: root.userListModel

            onResolveResource: (idOrUri, action) => root.resolveResource(idOrUri, action)
        }
    }

    Component {
        id: roomMedia
        RoomMedia {
            room: root.room
            mediaMessageFilterModel: root.mediaMessageFilterModel
        }
    }

    footer: Kirigami.NavigationTabBar {
        id: navigationBar
        visible: !root.room.isSpace
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false

        actions: [
            Kirigami.Action {
                id: infoAction
                text: i18n("Information")
                icon.name: "documentinfo"
                onTriggered: drawerItemLoader.sourceComponent = roomInformation
            },
            Kirigami.Action {
                text: i18n("Media")
                icon.name: "mail-attachment-symbollic"
                onTriggered: drawerItemLoader.sourceComponent = roomMedia
            }
        ]
    }

    Connections {
        target: root.Kirigami.PageStack.pageStack
        function onWideModeChanged(): void {
            if ((root.Kirigami.PageStack.pageStack as Kirigami.PageRow).wideMode) {
                root.Kirigami.PageStack.pop();
            }
        }
    }

    onBackRequested: event => {
        event.accepted = true;
        Kirigami.PageStack.pop();
    }
}
