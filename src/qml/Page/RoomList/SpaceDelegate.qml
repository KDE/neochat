// SPDX-FileCopyrightText: 2022 Snehit Sah <snehitsah@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.neochat 1.0

QQC2.ItemDelegate {
    id: root

    required property string avatar
    required property var currentRoom
    required property int index
    required property string id

    signal createContextMenu(currentRoom: var)
    signal spaceSelected(spaceId: string)

    height: ListView.view.height
    width: height

    leftPadding: topPadding
    rightPadding: topPadding

    contentItem: Kirigami.Avatar {
        name: currentRoom.displayName
        source: avatar !== "" ? "image://mxc/" + avatar : ""
    }

    onClicked: root.spaceSelected(id)
    onPressAndHold: root.createContextMenu(root.currentRoom)

    Accessible.name: currentRoom.displayName

    QQC2.ToolTip.text: currentRoom.displayName
    QQC2.ToolTip.visible: hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse
        onTapped: root.createContextMenu(root.currentRoom)
    }
}
