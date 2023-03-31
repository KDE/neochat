// SPDX-FileCopyrightText: 2022 Snehit Sah <snehitsah@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.neochat 1.0

ListView {
    id: root

    required property RoomListModel roomListModel

    orientation: Qt.Horizontal
    spacing: Kirigami.Units.smallSpacing
    clip: true
    visible: root.count > 0

    model: SortFilterSpaceListModel {
        id: sortFilterSpaceListModel
        sourceModel: root.roomListModel
    }

    header: QQC2.ItemDelegate {
        id: homeButton
        icon.name: "home"
        text: i18nc("@action:button", "Show All Rooms")
        height: parent.height
        width: height
        leftPadding: topPadding
        rightPadding: topPadding

        contentItem: Kirigami.Icon {
            source: "home"
        }

        onClicked: {
            sortFilterRoomListModel.activeSpaceId = "";
            listView.positionViewAtIndex(0, ListView.Beginning);
        }

        QQC2.ToolTip.text: homeButton.text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    delegate: SpaceDelegate {
        onSpaceSelected: (spaceId) => {
            sortFilterRoomListModel.activeSpaceId = spaceId;
        }

        onCreateContextMenu: () => {
            const menu = spaceListContextMenu.createObject(page, {
                room: currentRoom,
            });
            menu.open();
        }
    }

    Layout.preferredHeight: Kirigami.Units.gridUnit * 2
    Layout.fillWidth: true
}
