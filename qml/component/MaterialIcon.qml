import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Matrique.Settings 0.1

Item {
    property alias icon: iconText.text
    property var color: MSettings.darkTheme ? "white" : "black"

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
