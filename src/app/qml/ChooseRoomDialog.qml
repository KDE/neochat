// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 Azhar Momin <azhar.momin@kdemail.net>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

import org.kde.neochat

SearchPage {
    id: root

    title: i18nc("@title", "Choose a Room")

    showSearchButton: false

    signal chosen(list<string> roomIds)

    property bool multiple: false
    property string actionText: i18nc("@action", "Select")
    property string icon: "dialog-ok-symbolic"
    required property NeoChatConnection connection

    property var selectedRooms: []

    model: RoomManager.sortFilterRoomListModel
    modelDelegate: RoomDelegate {
        id: roomDelegate

        leading: root.multiple ? roomDelegateCheckbox : null

        onClicked: selectRoom()
        connection: root.connection
        openOnClick: false
        showConfigure: false

        function selectRoom() {
            if (!root.multiple) {
                root.chosen([currentRoom.id]);
                root.Kirigami.PageStack.closeDialog();
                return;
            }

            if (root.selectedRooms.includes(currentRoom.id)) {
                root.selectedRooms = root.selectedRooms.filter(id => id !== currentRoom.id)
            } else {
                root.selectedRooms = [...root.selectedRooms, currentRoom.id]
            }
        }

        Component {
            id: roomDelegateCheckbox

            QQC2.CheckBox {
                checked: root.selectedRooms.includes(roomDelegate.currentRoom.id)
                onToggled: roomDelegate.selectRoom()
            }
        }
    }

    footer: QQC2.ToolBar {
        visible: root.multiple
        QQC2.DialogButtonBox {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: root.actionText
                icon.name: root.icon
                enabled: root.selectedRooms.length > 0
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    root.chosen(root.selectedRooms);
                    root.Kirigami.PageStack.closeDialog();
                }
            }
            QQC2.Button {
                icon.name: "dialog-cancel-symbolic"
                text: i18nc("@action", "Cancel")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: root.Kirigami.PageStack.closeDialog()
            }
        }
    }
}
