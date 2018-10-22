import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
    property var row: null
    property var model: null
    property string selectedText

    readonly property bool isFile: model  && (model.eventType === "video" || model.eventType === "audio" || model.eventType === "file" || model.eventType === "image")

    id: messageContextMenu

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
        height: visible ? undefined : 0
        text: "Reply"

        onTriggered: {
            roomPanelInput.clear()
            roomPanelInput.isReply = true
            roomPanelInput.replyUserID = model.author.id
            roomPanelInput.replyEventID = model.eventId
            roomPanelInput.replyContent = selectedText != "" ? selectedText : model.message
        }
    }
    MenuItem {
        visible: model && model.author === currentRoom.localUser
        height: visible ? undefined : 0
        text: "Redact"

        onTriggered: currentRoom.redactEvent(model.eventId)
    }
}
