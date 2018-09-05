import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
    readonly property bool isFile: eventType === "video" || eventType === "audio" || eventType === "file" || eventType === "image"

    id: messageContextMenu

    MenuItem {
        text: "Copy"

        onTriggered: matriqueController.copyToClipboard(plainText)
    }
    MenuItem {
        text: "Copy Source"

        onTriggered: matriqueController.copyToClipboard(toolTip)
    }
    MenuItem {
        visible: isFile
        height: visible ? undefined : 0
        text: "Open Externally"

        onTriggered: messageRow.openExternally()
    }
    MenuItem {
        visible: isFile
        height: visible ? undefined : 0
        text: "Save As"

        onTriggered: messageRow.saveFileAs()
    }
    MenuItem {
        visible: sentByMe
        height: visible ? undefined : 0
        text: "Redact"

        onTriggered: currentRoom.redactEvent(eventId)
    }

    Component.onCompleted: popup()
    onClosed: messageContextMenu.destroy()
}
