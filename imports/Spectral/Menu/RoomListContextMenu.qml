import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

Menu {
    property var room

    id: root

    MenuItem {
        text: "Favourite"
        checkable: true
        checked: room.isFavourite

        onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
    }

    MenuItem {
        text: "Deprioritize"
        checkable: true
        checked: room.isLowPriority

        onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", 1.0)
    }

    MenuSeparator {}

    MenuItem {
        text: "Mark as Read"

        onTriggered: room.markAllMessagesAsRead()
    }

    MenuItem {
        text: "Leave Room"
        Material.foreground: Material.Red

        onTriggered: room.forget()
    }

    onClosed: destroy()
}
