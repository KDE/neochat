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

        QQC2.TabButton {
            text: qsTr("Room Data")
        }
        QQC2.TabButton {
            text: qsTr("Server Info")
        }
    }

    StackLayout {
        id: swipeView

        currentIndex: tabBar.currentIndex

        RoomData {
            room: root.room
            connection: root.connection
        }
        ServerData {
            connection: root.connection
        }
    }
}
