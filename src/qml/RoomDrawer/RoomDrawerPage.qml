// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.delegates 1.0 as Delegates
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents
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
Kirigami.ScrollablePage {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    readonly property NeoChatRoom room: RoomManager.currentRoom

    title: roomInformation.title

    actions {
        main: Kirigami.Action {
            icon.name: "settings-configure"
            onTriggered: applicationWindow().pageStack.pushDialogLayer('qrc:/Categories.qml', {room: root.room}, { title: i18n("Room Settings") })
        }
    }

    RoomInformation {
        id: roomInformation
        room: root.room
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
