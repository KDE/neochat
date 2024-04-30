// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection

    title: i18n("Developer Tools")

    leftPadding: 0
    rightPadding: 0

    header: QQC2.TabBar {
        id: tabBar

        readonly property real tabWidth: tabBar.width / tabBar.count

        QQC2.TabButton {
            text: i18nc("@title:tab", "Debug Options")

            implicitWidth: tabBar.tabWidth
        }
        QQC2.TabButton {
            text: i18nc("@title:tab", "Room Data")

            implicitWidth: tabBar.tabWidth
        }
        QQC2.TabButton {
            text: i18nc("@title:tab", "Server Info")

            implicitWidth: tabBar.tabWidth
        }
        QQC2.TabButton {
            text: i18nc("@title:tab", "Account Data")

            implicitWidth: tabBar.tabWidth
        }
        QQC2.TabButton {
            text: i18nc("@title:tab", "Feature Flags")

            implicitWidth: tabBar.tabWidth
        }
    }

    StackLayout {
        id: swipeView

        currentIndex: tabBar.currentIndex

        DebugOptions {}
        RoomData {
            room: root.room
            connection: root.connection
        }
        ServerData {
            connection: root.connection
        }
        AccountData {
            connection: root.connection
        }
        FeatureFlagPage {}
    }
}
