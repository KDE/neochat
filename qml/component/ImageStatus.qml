import QtQuick 2.10
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0

Image {
    id: avatar

    mipmap: true
    layer.enabled: true
    fillMode: Image.PreserveAspectCrop

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
        id: circle
        width: avatar.width
        height: avatar.width
        radius: avatar.width / 2
        color: "transparent"
        border.color: "#4caf50"
        border.width: 4
    }
}
