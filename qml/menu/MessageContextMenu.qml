import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
    property var row
    property bool canRedact
    property string eventType
    property string plainText
    property string toolTip
    property string eventId

    readonly property bool isFile: eventType === "video" || eventType === "audio" || eventType === "file" || eventType === "image"

    id: messageContextMenu

    MenuItem {
        text: "Copy"

        onTriggered: matriqueController.copyToClipboard(plainText)
    }
    MenuItem {
        text: "View Source"

        onTriggered: {
            sourceDialog.sourceText = toolTip
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
        visible: canRedact
        height: visible ? undefined : 0
        text: "Redact"

        onTriggered: currentRoom.redactEvent(eventId)
    }
}
