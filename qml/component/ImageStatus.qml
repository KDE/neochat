import QtQuick 2.10
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0

Item {
    property bool statusIndicator: false
    property bool opaqueBackground: false
    property alias source: avatar.source

    Rectangle {
        width: parent.width
        height: parent.width
        radius: parent.width / 2
        color: "white"
        visible: opaqueBackground
    }

    Image {
        id: avatar
        width: parent.width
        height: parent.width

        mipmap: true
        layer.enabled: true
        fillMode: Image.PreserveAspectCrop
        sourceSize.width: parent.width
        sourceSize.height: parent.width

        layer.effect: OpacityMask {
            maskSource: Item {
                width: avatar.width
                height: avatar.width
                Rectangle {
                    anchors.centerIn: parent
                    width: avatar.width
                    height: avatar.width
                    radius: avatar.width / 2
                }
            }
        }

        Rectangle {
            width: parent.width
            height: parent.width
            radius: parent.width / 2
            color: "transparent"
            border.color: "#4caf50"
            border.width: 4
            visible: statusIndicator
        }
    }
}
