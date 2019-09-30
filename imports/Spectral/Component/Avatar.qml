import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

import Spectral.Setting 0.1

Item {
    property string hint: "H"
    property string source: ""
    property color color: MPalette.accent
    readonly property url realSource: source ? "image://mxc/" + source : ""

    id: root

    Image {
        anchors.fill: parent

        id: image
        visible: realSource
        source: width < 1 ? "" : realSource
        sourceSize.width: width
        sourceSize.height: width
        fillMode: Image.PreserveAspectCrop
        mipmap: true
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: image.width
                height: image.width

                radius: width / 2
            }
        }
    }

    Rectangle {
        anchors.fill: parent

        visible: !realSource || image.status != Image.Ready

        radius: height / 2
        color: root.color
        antialiasing: true

        Label {
            anchors.centerIn: parent

            color: "white"
            text: hint[0].toUpperCase()
            font.pixelSize: root.width / 2
            font.weight: Font.Medium
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
