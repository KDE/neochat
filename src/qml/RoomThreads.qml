// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami

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
     * @brief The item to the left of the title.
     *
     * Setting Enabled dictates the visibility of the item (i.e. parent visibility
     * is set false when Enabled is false).
     */
    property Item leading: QQC2.ToolButton {
        id: backButton
        enabled: false

        icon.name: "arrow-left"
        text: i18n("Back to thread list")
        display: QQC2.AbstractButton.IconOnly

        onClicked: {
            root.pop()
            enabled = false;
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.visible: hovered
    }

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom
    onCurrentRoomChanged: root.pop()

    initialItem: QQC2.ScrollView {
        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ListView {
            // So that delegates can access current room properly.
            readonly property NeoChatRoom currentRoom: root.currentRoom

            bottomMargin: Kirigami.Units.largeSpacing
            verticalLayoutDirection: ListView.BottomToTop
            clip: true

            model: RoomThreadsModel {
                room: root.currentRoom
            }

            delegate: ThreadRootDelegate {
                onClicked: {
                    let view = threadPage.createObject(this, {model: threadModel});
                    root.push(view);
                    backButton.enabled = true;
                }
            }
        }
    }

    Component {
        id: threadPage
        ThreadPage {
            room: root.currentRoom
        }
    }
}
