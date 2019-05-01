import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    property string eventId
    property url localPath

    id: root

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visible: true
    visibility: Qt.WindowFullScreen

    title: "Image View - " + eventId

    color: "#BB000000"

    Shortcut {
        sequence: "Escape"
        onActivated: root.destroy()
    }

    AnimatedImage {
        anchors.centerIn: parent

        width: Math.min(sourceSize.width, root.width)
        height: Math.min(sourceSize.height, root.height)

        fillMode: Image.PreserveAspectFit
        cache: false

        source: localPath
    }

    ItemDelegate {
        anchors.top: parent.top
        anchors.right: parent.right

        width: 64
        height: 64

        contentItem: MaterialIcon {
            icon: "\ue5cd"
            color: "white"
        }

        onClicked: root.destroy()
    }
}
