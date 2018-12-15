import QtQuick 2.12
import QtQuick.Controls 2.4
import QtGraphicalEffects 1.0

Item {
    property string hint: "H"
    property string source: ""
    readonly property url realSource: source ? "image://mxc/" + source : ""

    id: root

    Image {
        anchors.fill: parent

        id: image
        visible: realSource
        source: realSource
        sourceSize.width: width
        sourceSize.height: width
        fillMode: Image.PreserveAspectCrop
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

        color: stringToColor(hint)

        Label {
            anchors.centerIn: parent

            color: "white"
            text: hint[0].toUpperCase()
            font.pixelSize: root.width / 2
            font.bold: true
        }
    }

    function stringToColor(str) {
        var hash = 0;
        for (var i = 0; i < str.length; i++) {
            hash = str.charCodeAt(i) + ((hash << 5) - hash);
        }
        var colour = '#';
        for (var j = 0; j < 3; j++) {
            var value = (hash >> (j * 8)) & 0xFF;
            colour += ('00' + value.toString(16)).substr(-2);
        }
        return colour;
    }
}
