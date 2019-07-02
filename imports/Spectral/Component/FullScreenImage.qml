import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    property string filename
    property url localPath

    id: root

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visible: true
    visibility: Qt.WindowFullScreen

    title: "Image View - " + filename

    color: "#BB000000"

    Shortcut {
        sequence: "Escape"
        onActivated: root.destroy()
    }

    AnimatedImage {
        anchors.centerIn: parent

        width: Math.min(sourceSize.width, root.width)
        height: Math.min(sourceSize.height, root.height)

        cache: false
        fillMode: Image.PreserveAspectFit

        source: localPath
    }

    ItemDelegate {
        anchors.top: parent.top
        anchors.right: parent.right

        id: closeButton

        width: 64
        height: 64

        contentItem: MaterialIcon {
            icon: "\ue5cd"
            color: "white"
        }

        onClicked: root.destroy()
    }
}
