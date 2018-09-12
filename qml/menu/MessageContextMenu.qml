import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
    property var row: null
    property var model: null

    readonly property bool isFile: model  && (model.eventType === "video" || model.eventType === "audio" || model.eventType === "file" || model.eventType === "image")

    id: messageContextMenu

    MenuItem {
        text: "Copy"

        onTriggered: matriqueController.copyToClipboard(model.plainText)
    }
    MenuItem {
        text: "View Source"

        onTriggered: {
            sourceDialog.sourceText = model.toolTip
            sourceDialog.open()
        }
    }
    MenuItem {
        visible: isFile
        height: visible ? undefined : 0
        text: "Open Externally"

        onTriggered: row.openExternally()
    }
    MenuItem {
        visible: isFile
        height: visible ? undefined : 0
        text: "Save As"

        onTriggered: row.saveFileAs()
    }
    MenuItem {
        visible: model && model.author === currentRoom.localUser
        height: visible ? undefined : 0
        text: "Redact"

        onTriggered: currentRoom.redactEvent(model.eventId)
    }
}
