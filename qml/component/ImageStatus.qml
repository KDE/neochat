import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.2

Item {
    property bool round: true
    property string source: ""
    property string displayText: ""
    readonly property bool showImage: source
    readonly property bool showInitial: !showImage && displayText  || avatar.status != Image.Ready

    id: item

    Image {
        width: item.width
        height: item.width

        id: avatar

        visible: showImage
        source: item.source

        mipmap: true
        layer.enabled: true
        fillMode: Image.PreserveAspectCrop
        sourceSize.width: item.width

        layer.effect: OpacityMask {
            maskSource: Item {
                width: avatar.width
                height: avatar.width
                Rectangle {
                    anchors.centerIn: parent
                    width: avatar.width
                    height: avatar.width
                    radius: round? avatar.width / 2 : 0
                }
            }
        }
    }

    Label {
        anchors.fill: parent

        color: "white"
        visible: showInitial
        text: showInitial ? getInitials(displayText)[0] : ""
        font.pixelSize: 22
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        background: Rectangle {
            anchors.fill: parent
            radius: round? width / 2 : 0
            color: showInitial ? stringToColor(displayText) : Material.accent
        }
    }

    function getInitials(text) {
        if (!text) return "N"
        var initial = text.toUpperCase().replace(/[^a-zA-Z- ]/g, "").match(/\b\w/g);
        if (!initial) return "N"
        return initial
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
