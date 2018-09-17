import QtQuick 2.9
import QtQuick.Controls 2.2
import Spectral 0.1

Menu {
    property var model: null

    id: roomListMenu

    MenuItem {
        text: "Favourite"
        checkable: true
        checked: model && model.category === RoomType.Favorite

        onTriggered: model.category === RoomType.Favorite ? model.currentRoom.removeTag("m.favourite") : model.currentRoom.addTag("m.favourite", "1")
    }
    MenuItem {
        text: "Deprioritize"
        checkable: true
        checked: model && model.category === RoomType.Deprioritized

        onTriggered: model.category === RoomType.Deprioritized ? model.currentRoom.removeTag("m.lowpriority") : model.currentRoom.addTag("m.lowpriority", "1")
    }
    MenuSeparator {}
    MenuItem {
        text: "Mark as Read"

        onTriggered: model.currentRoom.markAllMessagesAsRead()
    }
    MenuItem {
        text: "Leave Room"

        onTriggered: model.currentRoom.forget()
    }
}
