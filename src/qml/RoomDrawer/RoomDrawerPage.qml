// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

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
    readonly property NeoChatRoom room: RoomManager.currentRoom

    title: drawerItemLoader.item ? drawerItemLoader.item.title : ""

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component.onCompleted: infoAction.toggle()

    actions {
        main: Kirigami.Action {
            displayHint: Kirigami.DisplayHint.IconOnly
            text: i18n("Settings")
            icon.name: "settings-configure"
            onTriggered: applicationWindow().pageStack.pushDialogLayer('qrc:/Categories.qml', {room: root.room}, { title: i18n("Room Settings") })
        }
    }

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
        }
    }

    Component {
        id: roomMedia
        RoomMedia {
            currentRoom: root.room
        }
    }

    footer: Kirigami.NavigationTabBar {
        id: navigationBar
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
        target: applicationWindow().pageStack
        onWideModeChanged: {
            if (applicationWindow().pageStack.wideMode) {
                console.log("widemode pop")
                applicationWindow().pageStack.pop()
            }
        }
    }

    onBackRequested: event => {
        event.accepted = true;
        applicationWindow().pageStack.pop()
    }
}
