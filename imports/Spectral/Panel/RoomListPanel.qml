import QtQuick 2.9
import QtQuick.Controls 2.2

import SortFilterProxyModel 0.2

RoomListPanelForm {
    model: sortedRoomListModel

    SortFilterProxyModel {
        id: sortedRoomListModel

        sourceModel: listModel

        proxyRoles: ExpressionRole {
            name: "display"
            expression: {
                switch (category) {
                case 1: return "Invited"
                case 2: return "Favorites"
                case 3: return "Rooms"
                case 4: return "People"
                case 5: return "Low Priority"
                }
            }
        }

        sorters: [
            RoleSorter { roleName: "category" },
            RoleSorter {
                roleName: "lastActiveTime"
                sortOrder: Qt.DescendingOrder
            }
        ]

        filters: [
            RegExpFilter {
                roleName: "name"
                pattern: searchField.text
                caseSensitivity: Qt.CaseInsensitive
            },
            ExpressionFilter {
                enabled: filter === 1
                expression: unreadCount > 0
            },
            ExpressionFilter {
                enabled: filter === 2
                expression: category === 1 || category === 2 || category === 4
            },
            ExpressionFilter {
                enabled: filter === 3
                expression: category === 3 || category === 5
            }
        ]
    }

    Shortcut {
        sequence: StandardKey.Find
        onActivated: searchField.forceActiveFocus()
    }

    Dialog {
        property var currentRoom

        id: inviteDialog
        parent: ApplicationWindow.overlay

        x: (window.width - width) / 2
        y: (window.height - height) / 2
        width: 360

        title: "Action Required"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: Label { text: "Accept this invitation?" }

        onAccepted: currentRoom.acceptInvitation()
        onRejected: currentRoom.forget()
    }
}
