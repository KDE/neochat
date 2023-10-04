// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.neochat

/**
 * @brief Component for visualising the threads in the room.
 *
 * The component is a list of threads with the ability to select a thread and view
 * all messages in it.
 *
 * @note This component is only the contents, it will need to be placed in either
 *       a drawer (desktop) or page (mobile) to be used.
 *
 * @sa RoomDrawer, RoomDrawerPage
 */
QQC2.StackView {
    id: root

    /**
     * @brief The title that should be displayed for this component if available.
     */
    readonly property string title: i18nc("@action:title", "Room Threads")

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    required property NeoChatConnection connection

    initialItem: QQC2.ScrollView {
        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ListView {
            // So that delegates can access current room properly.
            readonly property NeoChatRoom currentRoom: root.currentRoom

            clip: true
            verticalLayoutDirection: ListView.BottomToTop

            model: RoomThreadsModel {
                room: root.currentRoom
            }

            delegate: ThreadRootDelegate {}
        }
    }
}
