import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3

Item {
    property alias icon: iconText.text
    property var color: Material.theme == Material.Light ? "black" : "white"

    id: item

    Text {
        id: iconText
        anchors.fill: parent
        font.pointSize: 16
        font.family: materialFont.name
        color: item.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
   }
}
