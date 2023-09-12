// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

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
