// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

Kirigami.ScrollablePage {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection
    property alias currentTabIndex: tabBar.currentIndex

    title: i18nc("@title", "Developer Tools")

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

    Loader {
        sourceComponent: switch (tabBar.currentIndex) {
            case 0: return debugOptions;
            case 1: return roomData;
            case 2: return serverData;
            case 3: return accountData;
            case 4: return featureFlags;
        }
    }

    Component {
        id: debugOptions
        DebugOptions {}
    }

    Component {
        id: roomData
        RoomData {
            room: root.room
            connection: root.connection
        }
    }

    Component {
        id: serverData
        ServerData {
            connection: root.connection
        }
    }

    Component {
        id: accountData
        AccountData {
            connection: root.connection
        }
    }

    Component {
        id: featureFlags
        FeatureFlagPage {}
    }
}
