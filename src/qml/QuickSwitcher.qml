// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.neochat

Kirigami.SearchDialog {
    id: root

    required property NeoChatConnection connection

    Shortcut {
        sequence: "Ctrl+K"
        onActivated: root.open()
    }

    onAccepted: if (currentItem) {
        currentItem.clicked();
    }

    onTextChanged: RoomManager.sortFilterRoomListModel.filterText = text
    model: RoomManager.sortFilterRoomListModel
    emptyText: i18nc("Placeholder message", "No room found")
    parent: QQC2.Overlay.overlay

    delegate: RoomDelegate {
        connection: root.connection
        onClicked: root.close()
        showConfigure: false
    }
}
