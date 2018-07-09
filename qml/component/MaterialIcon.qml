import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Controls.Material 2.4

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
