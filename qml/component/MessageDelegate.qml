import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

Item {
    id: messageDelegate

    readonly property bool sentByMe: author === currentRoom.localUser

    width: delegateLoader.width
    height: delegateLoader.height

    anchors.right: (eventType === "message" || eventType === "image" || eventType === "file" || eventType === "video" || eventType === "audio" || eventType === "notice") && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: (eventType === "state" || eventType === "emote") ? parent.horizontalCenter : undefined

    MouseArea {
        anchors.fill: parent

        ToolTip.visible: pressed
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
        ToolTip.text: time
    }

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
