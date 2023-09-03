// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

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
    required property NeoChatRoom currentRoom

    // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
    QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

    ListView {
        // So that delegates can access current room properly.
        readonly property NeoChatRoom currentRoom: root.currentRoom

        clip: true
        verticalLayoutDirection: ListView.BottomToTop

        model: RoomManager.mediaMessageFilterModel

        delegate: DelegateChooser {
            role: "type"

            DelegateChoice {
                roleValue: 0//MediaMessageFilterModel.Image
                delegate: ImageDelegate {
                    alwaysShowAuthor: true
                    alwaysMaxWidth: true
                    cardBackground: false
                }
            }

            DelegateChoice {
                roleValue: 1//MediaMessageFilterModel.Video
                delegate: VideoDelegate {
                    alwaysShowAuthor: true
                    alwaysMaxWidth: true
                    cardBackground: false
                }
            }
        }
    }
}
