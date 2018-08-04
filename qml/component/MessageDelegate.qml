import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

Item {
    id: messageDelegate

    readonly property bool darkTheme: Material.theme == Material.Dark
    readonly property color background: darkTheme ? "#242424" : "lightgrey"

    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isState: eventType === "state" || eventType === "emote"
    readonly property bool isMessage: eventType === "message" || eventType === "notice"
    readonly property bool isFile: eventType === "video" || eventType === "audio" || eventType === "file" || eventType === "image"

    visible: eventType != "redaction"

    z: -5
    width: delegateLoader.width
    height: delegateLoader.height

    anchors.right: !isState && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: isState ? parent.horizontalCenter : undefined

    MouseArea {
        anchors.fill: parent
        onPressAndHold: messageContextMenu.popup()

        Menu {
            id: messageContextMenu
            MenuItem {
                text: "Redact"
                onTriggered: currentRoom.redactEvent(eventId)
            }
        }
    }

    Loader {
        id: delegateLoader

        source: eventType != "redaction" ? isMessage ? "MessageBubble.qml" : isState ? "StateBubble.qml" : isFile ? eventType === "image" ? "ImageBubble.qml" : "FileBubble.qml" : "" : ""
    }
}
