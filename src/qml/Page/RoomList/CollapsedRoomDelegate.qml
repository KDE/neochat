// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

import './' as RoomList

QQC2.ItemDelegate {
    id: root

    required property var currentRoom
    required property bool categoryVisible
    required property string filterText
    required property string avatar
    required property string name

    topPadding: Kirigami.Units.largeSpacing
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    width: ListView.view.width
    height: visible ? ListView.view.width : 0

    visible: root.categoryVisible || filterText.length > 0 || Config.mergeRoomList

    contentItem: Kirigami.Avatar {
        source: root.avatar ? `image://mxc/${root.avatar}` : ""
        name: root.name || i18n("No Name")

        sourceSize {
            width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
        }
    }

    onClicked: RoomManager.enterRoom(root.currentRoom)

    Keys.onEnterPressed: RoomManager.enterRoom(root.currentRoom)
    Keys.onReturnPressed: RoomManager.enterRoom(root.currentRoom)

    QQC2.ToolTip.visible: text.length > 0 && hovered
    QQC2.ToolTip.text: root.name ?? ""
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
}
