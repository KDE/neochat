import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import Matrique 0.1
import Matrique.Settings 0.1

Item {
    readonly property bool hidden: marks === EventStatus.Redacted || marks === EventStatus.Hidden
    readonly property color background: MSettings.darkTheme ? "#242424" : "lightgrey"
    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isState: eventType === "state" || eventType === "emote"

    id: messageDelegate

    z: -5
    width: delegateLoader.width
    height: delegateLoader.height

    anchors.right: !isState && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: isState ? parent.horizontalCenter : undefined

    AutoMouseArea {
        anchors.fill: parent

        onSecondaryClicked: Qt.createComponent("qrc:/qml/menu/MessageContextMenu.qml").createObject(this)
    }

    Loader {
        id: delegateLoader

        asynchronous: MSettings.asyncMessageDelegate

        source: {
            if (eventType == "redaction" || hidden) return ""
            switch (eventType) {
            case "state":
            case "emote":
                return "StateBubble.qml"
            case "message":
            case "notice":
                return "MessageBubble.qml"
            case "image":
                return "ImageBubble.qml"
            case "audio":
                return "AudioBubble.qml"
            case "video":
            case "file":
                return "FileBubble.qml"
            }
            return ""
        }
    }
}
