// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.ApplicationWindow {
    id: root

    required property NeoChatRoom currentRoom
    required property NeoChatConnection connection

    minimumWidth: Kirigami.Units.gridUnit * 10
    minimumHeight: Kirigami.Units.gridUnit * 15

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: root.close()
    }
    pageStack.initialPage: RoomPage {
        id: roomPage
        visible: true
        currentRoom: root.currentRoom
        disableCancelShortcut: true
        connection: root.connection

        timelineModel: TimelineModel {
            room: currentRoom
        }
        messageFilterModel: MessageFilterModel {
            sourceModel: roomPage.messageEventModel
        }
        mediaMessageFilterModel: MediaMessageFilterModel {
            sourceModel: roomPage.messageFilterModel
        }
    }

    onCurrentRoomChanged: if (!currentRoom) {
        root.close();
    }

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
