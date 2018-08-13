import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import Matrique 0.1

Item {
    id: messageDelegate

    readonly property bool darkTheme: Material.theme == Material.Dark
    readonly property color background: darkTheme ? "#242424" : "lightgrey"

    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isState: eventType === "state" || eventType === "emote"

    visible: marks !== EventStatus.Hidden

    z: -5
    width: delegateLoader.width
    height: delegateLoader.height

    anchors.right: !isState && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: isState ? parent.horizontalCenter : undefined

    MouseArea {
        anchors.fill: parent
        onPressAndHold: Qt.createComponent("MessageContextMenu.qml").createObject(this)
    }

    Loader {
        id: delegateLoader

        source: {
            if (eventType == "redaction") return ""
            switch (eventType) {
            case "state":
            case "emote":
                return "StateBubble.qml"
            case "message":
            case "notice":
                return "MessageBubble.qml"
            case "image":
                return "ImageBubble.qml"
            case "video":
            case "audio":
            case "file":
                return "FileBubble.qml"
            }
            return ""
        }
    }
}
