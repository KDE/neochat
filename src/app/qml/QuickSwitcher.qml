// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.SearchDialog {
    id: root

    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow window

    Shortcut {
        sequence: "Ctrl+K"
        onActivated: if (root.connection) root.open()
    }

    onAccepted: if (currentItem) {
        (currentItem as QQC2.ItemDelegate).clicked();
    }

    onTextChanged: RoomManager.sortFilterRoomListModel.filterText = text
    model: RoomManager.sortFilterRoomListModel
    emptyText: i18nc("Placeholder message", "No room found")
    Kirigami.Action {
        id: exploreRoomAction
        text: i18nc("@action:button", "Explore rooms")
        icon.name: "compass"
        onTriggered: {
            root.close()
            let dialog = root.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Explore Rooms")
            });
            dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
            });
        }
    }

    Component.onCompleted: emptyHelpfulAction = exploreRoomAction

    parent: QQC2.Overlay.overlay

    delegate: RoomDelegate {
        connection: root.connection
        onClicked: root.close()
        showConfigure: false
    }
}
