pragma Singleton
import QtQuick 2.12
import Qt.labs.settings 1.0

Settings {
    property bool showNotification: true

    property bool showTray: true

    property bool darkTheme

    property string fontFamily: "Roboto,Noto Sans,Noto Color Emoji"

    property bool markdownFormatting: true
}
