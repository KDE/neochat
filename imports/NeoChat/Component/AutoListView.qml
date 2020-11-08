import QtQuick 2.12

ListView {
    pixelAligned: true

    ScrollHelper {
        anchors.fill: parent

        flickable: parent
    }
}
