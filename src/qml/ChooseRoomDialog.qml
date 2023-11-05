// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title", "Choose a Room")

    signal chosen(string roomId)

    required property NeoChatConnection connection

    header: Kirigami.SearchField {
        onTextChanged: sortModel.filterText = text
    }

    ListView {
        model: SortFilterRoomListModel {
            id: sortModel
            sourceModel: RoomListModel {
                connection: root.connection
            }
        }
        delegate: RoomDelegate {
            id: roomDelegate
            filterText: ""
            onSelected: {
                root.chosen(roomDelegate.currentRoom.id)
            }
            connection: root.connection
        }
    }
}
