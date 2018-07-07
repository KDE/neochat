import QtQuick 2.11
import QtQuick.Controls 2.4
import QtGraphicalEffects 1.0

Item {
    property bool opaqueBackground: false
    property alias source: avatar.source

    id: item

    Rectangle {
        width: item.width
        height: item.width
        radius: item.width / 2
        color: "white"
        visible: opaqueBackground
    }

    Image {
        id: avatar
        width: item.width
        height: item.width

        mipmap: true
        layer.enabled: true
        fillMode: Image.PreserveAspectCrop
        sourceSize.width: item.width
        sourceSize.height: item.width

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
    }
}
