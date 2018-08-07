import QtQuick 2.9
import QtQuick.Controls 2.2

Menu {
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
        onTriggered: delegateLoader.item.downloadAndOpen()
    }
    MenuItem {
        visible: isFile
        height: visible ? undefined : 0
        text: "Save As"
        onTriggered: delegateLoader.item.saveFileAs()
    }
    MenuItem {
        visible: sentByMe
        height: visible ? undefined : 0
        text: "Redact"
        onTriggered: currentRoom.redactEvent(eventId)
    }

    Component.onCompleted: popup()
}
