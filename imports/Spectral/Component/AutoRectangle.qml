import QtQuick 2.12

Rectangle {
    property alias topLeftRadius: topLeftRect.radius
    property alias topRightRadius: topRightRect.radius
    property alias bottomLeftRadius: bottomLeftRect.radius
    property alias bottomRightRadius: bottomRightRect.radius

    property alias topLeftVisible: topLeftRect.visible
    property alias topRightVisible: topRightRect.visible
    property alias bottomLeftVisible: bottomLeftRect.visible
    property alias bottomRightVisible: bottomRightRect.visible

    antialiasing: true

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left

        width: parent.width / 2
        height: parent.height / 2

        id: topLeftRect

        antialiasing: true

        color: parent.color
    }

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right

        width: parent.width / 2
        height: parent.height / 2

        id: topRightRect

        antialiasing: true

        color: parent.color
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        width: parent.width / 2
        height: parent.height / 2

        id: bottomLeftRect

        antialiasing: true

        color: parent.color
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        width: parent.width / 2
        height: parent.height / 2

        id: bottomRightRect

        antialiasing: true

        color: parent.color
    }
}
