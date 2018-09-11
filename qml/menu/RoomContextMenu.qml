import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
    property var room: null

    id: roomListMenu

    MenuItem {
        text: "Favourite"
        checkable: true
        checked: room && room.isFavourite

        onTriggered: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", "1")
    }
    MenuItem {
        text: "Deprioritize"
        checkable: true
        checked: room && room.isLowPriority

        onTriggered: room.isLowPriority ? room.removeTag("m.lowpriority") : room.addTag("m.lowpriority", "1")
    }
    MenuSeparator {}
    MenuItem {
        text: "Mark as Read"

        onTriggered: room.markAllMessagesAsRead()
    }
    MenuItem {
        text: "Leave Room"

        onTriggered: room.forget()
    }
}
