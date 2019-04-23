import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    property url imageUrl
    property int sourceWidth
    property int sourceHeight

    id: root

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visible: true
    visibility: Qt.WindowFullScreen

    title: "Image View - " + imageUrl

    color: "#BB000000"

    Image {
        anchors.centerIn: parent

        sourceSize.width: root.sourceWidth
        sourceSize.height: root.sourceHeight
        source: imageUrl
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
