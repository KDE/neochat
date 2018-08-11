import QtQuick 2.9
import QtQuick.Controls 2.2

Text {
    property string category

    width: 36
    height: 36

    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter

    font.pointSize: 20
    font.family: "Noto Color Emoji"

    MouseArea {
        anchors.fill: parent
        onClicked: emojiCategory = category
    }
}
