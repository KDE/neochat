// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.neochat.libneochat
import org.kde.neochat.timeline as Timeline

/**
 * @brief Component for visualising the loaded media items in the room.
 *
 * The component is a simple list of media delegates (videos or images) with the
 * ability to open them in the mamimize component.
 *
 * @note This component is only the contents, it will need to be placed in either
 *       a drawer (desktop) or page (mobile) to be used.
 *
 * @sa RoomDrawer, RoomDrawerPage
 */
QQC2.ScrollView {
    id: root

    /**
    * @brief The title that should be displayed for this component if available.
    */
    readonly property string title: i18nc("@action:title", "Room Media")

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom room

    required property Timeline.MediaMessageFilterModel mediaMessageFilterModel

    // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
    QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

    ListView {
        clip: true
        verticalLayoutDirection: ListView.BottomToTop

        model: root.mediaMessageFilterModel

        delegate: DelegateChooser {
            role: "type"

            DelegateChoice {
                roleValue: Timeline.MediaMessageFilterModel.Image
                delegate: Timeline.MessageDelegate {
                    alwaysFillWidth: true
                    cardBackground: false
                    room: root.room
                }
            }

            DelegateChoice {
                roleValue: Timeline.MediaMessageFilterModel.Video
                delegate: Timeline.MessageDelegate {
                    alwaysFillWidth: true
                    cardBackground: false
                    room: root.room
                }
            }
        }
    }
}
