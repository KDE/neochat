import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
    id: roomListMenu

    MenuItem {
        text: "Favourite"
        checkable: true
        checked: currentRoom && currentRoom.isFavourite
        onTriggered: currentRoom.isFavourite ? currentRoom.removeTag("m.favourite") : currentRoom.addTag("m.favourite", "1")
    }
    MenuItem {
        text: "Deprioritize"
        checkable: true
        checked: currentRoom && currentRoom.isLowPriority
        onTriggered: currentRoom.isLowPriority ? currentRoom.removeTag("m.lowpriority") : currentRoom.addTag("m.lowpriority", "1")
    }
    MenuSeparator {}
    MenuItem {
        text: "Mark as Read"
        onTriggered: currentRoom.markAllMessagesAsRead()
    }
    MenuItem {
        text: "Leave Room"
        onTriggered: currentRoom.forget()
    }

    Component.onCompleted: popup()
    onClosed: roomListMenu.destroy()
}
