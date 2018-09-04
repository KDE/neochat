import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import Matrique.Settings 0.1

Control {
    property bool highlighted: false
    property bool colored: false

    readonly property bool darkBackground: highlighted  ? true : MSettings.darkTheme
    readonly property color backgroundColor: MSettings.darkTheme ? "#242424" : "lightgrey"

    padding: 12

    background: Rectangle {
        color: colored ? Material.accent : highlighted ? Material.primary : backgroundColor
    }

    AutoMouseArea {
        anchors.fill: parent

        onSecondaryClicked: Qt.createComponent("qrc:/qml/menu/MessageContextMenu.qml").createObject(this)
    }
}
