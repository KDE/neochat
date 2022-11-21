// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

Kirigami.ApplicationWindow {
    id: window
    required property var currentRoom
    minimumWidth: Kirigami.Units.gridUnit * 10
    minimumHeight: Kirigami.Units.gridUnit * 15

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: window.close()
    }
    pageStack.initialPage: RoomPage {
        visible: true
        currentRoom: window.currentRoom
        disableCancelShortcut: true
    }

    onCurrentRoomChanged: if (!currentRoom) {
        window.close()

    property Item hoverLinkIndicator: QQC2.Control {
        parent: overlay.parent
        property string text
        opacity: linkText.text.length > 0 ? 1 : 0

        z: 20
        x: 0
        y: parent.height - implicitHeight
        contentItem: QQC2.Label {
            id: linkText
            text: parent.text.startsWith("https://matrix.to/") ? "" : parent.text
        }
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
             color: Kirigami.Theme.backgroundColor
        }
    }
}
