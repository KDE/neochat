// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

import "./RoomList"

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title", "Choose a Room")

    signal chosen(string roomId)

    header: Kirigami.SearchField {
        onTextChanged: sortModel.filterText = text
    }

    ListView {
        model: SortFilterRoomListModel {
            id: sortModel
            sourceModel: RoomListModel {
                connection: Controller.activeConnection
            }
        }
        delegate: RoomDelegate {
            id: roomDelegate
            filterText: ""
            onClicked: {
                root.chosen(roomDelegate.currentRoom.id)
            }
        }
    }
}
