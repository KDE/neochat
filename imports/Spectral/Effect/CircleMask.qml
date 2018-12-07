import QtQuick 2.12
import QtGraphicalEffects 1.0

Item {
    id: item

    property alias source: mask.source

    Rectangle {
        id: circleMask

        width: parent.width
        height: parent.height

        smooth: true
        visible: false

        radius: Math.max(width/2, height/2)
    }

    OpacityMask {
        id: mask

        width: parent.width
        height: parent.height

        maskSource: circleMask
    }
}
