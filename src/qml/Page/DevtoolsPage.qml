// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

Kirigami.Page {
    id: devtoolsPage

    property NeoChatRoom room

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
        anchors.fill: parent

        currentIndex: tabBar.currentIndex

        RoomData {}
        ServerData {}
    }
}
