// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kitemmodels

import org.kde.neochat
import org.kde.neochat.config

QQC2.ItemDelegate {
    id: root

    required property NeoChatRoom currentRoom
    required property bool categoryVisible
    required property string filterText
    required property string avatar
    required property string displayName

    topPadding: Kirigami.Units.largeSpacing
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    width: ListView.view.width
    height: visible ? ListView.view.width : 0

    visible: root.categoryVisible || filterText.length > 0

    contentItem: KirigamiComponents.Avatar {
        source: root.avatar ? `image://mxc/${root.avatar}` : ""
        name: root.displayName

        sourceSize {
            width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
        }
    }

    onClicked: RoomManager.resolveResource(currentRoom.id)

    Keys.onEnterPressed: RoomManager.resolveResource(currentRoom.id)
    Keys.onReturnPressed: RoomManager.resolveResource(currentRoom.id)

    QQC2.ToolTip.visible: text.length > 0 && hovered
    QQC2.ToolTip.text: root.displayName ?? ""
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
}
