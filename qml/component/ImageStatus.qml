import QtQuick 2.11
import QtQuick.Controls 2.4
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.4

Item {
    property bool opaqueBackground: false
    property bool round: true
    property string source: ""
    property string displayText: ""
    readonly property bool showImage: source
    readonly property bool showInitial: !showImage && displayText

    id: item

    Rectangle {
        width: item.width
        height: item.width
        radius: round ? item.width / 2 : 0
        color: "white"
        visible: opaqueBackground
    }

    Image {
        id: avatar
        width: item.width
        height: item.width
        visible: showImage
        source: item.source

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
        return text.toUpperCase().replace(/[^a-zA-Z- ]/g, "").match(/\b\w/g);
    }

    function stringToColor(str) {
        var hash = 0;
        for (var i = 0; i < str.length; i++) {
            hash = str.charCodeAt(i) + ((hash << 5) - hash);
        }
        var colour = '#';
        for (var i = 0; i < 3; i++) {
            var value = (hash >> (i * 8)) & 0xFF;
            colour += ('00' + value.toString(16)).substr(-2);
        }
        return colour;
    }
}
