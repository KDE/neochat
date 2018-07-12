import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

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
