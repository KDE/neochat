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
    readonly property bool isMessage: eventType === "message" || eventType === "notice"
    readonly property bool isFile: eventType === "video" || eventType === "audio" || eventType === "file" || eventType === "image"

    visible: marks !== EventStatus.Hidden

    z: -5
    width: delegateLoader.width
    height: delegateLoader.height

    anchors.right: !isState && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: isState ? parent.horizontalCenter : undefined

    MouseArea {
        anchors.fill: parent
        onPressAndHold: menuComponent.createObject(this)

        Component {
            id: menuComponent
            Menu {
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
        }
    }

    Loader {
        id: delegateLoader

        source: eventType != "redaction" ? isMessage ? "MessageBubble.qml" : isState ? "StateBubble.qml" : isFile ? eventType === "image" ? "ImageBubble.qml" : "FileBubble.qml" : "" : ""
    }
}
