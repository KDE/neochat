pragma Singleton
import QtQuick 2.12
import QtQuick.Controls.Material 2.12

QtObject {
    readonly property int theme: MSettings.darkTheme ? Material.Dark : Material.Light

    readonly property color primary: "#344955"
    readonly property color accent: "#673AB7"
    readonly property color foreground: MSettings.darkTheme ? "#FFFFFF" : "#1D333E"
    readonly property color background: MSettings.darkTheme ? "#303030" : "#FFFFFF"
    readonly property color lighter: MSettings.darkTheme ? "#FFFFFF" : "#5B7480"
    readonly property color banner: MSettings.darkTheme ? "#404040" : "#F2F3F4"
}
