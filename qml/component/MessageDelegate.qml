import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

Item {
    id: messageDelegate

    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool darkTheme: Material.theme == Material.Dark
    readonly property color background: darkTheme ? "#242424" : "lightgrey"

    z: -5
    width: delegateLoader.width
    height: delegateLoader.height

    anchors.right: !(eventType === "state" || eventType === "emote") && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: (eventType === "state" || eventType === "emote") ? parent.horizontalCenter : undefined

    Loader {
        id: delegateLoader

        source: {
            switch (eventType) {
            case "notice":
            case "message":
                return "MessageBubble.qml"
            case "image":
                return "ImageBubble.qml"
            case "emote":
            case "state":
                return "StateBubble.qml"
            case "video":
            case "audio":
            case "file":
                return "FileBubble.qml"
            }
            return ""
        }
    }
}
