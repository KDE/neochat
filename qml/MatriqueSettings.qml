pragma Singleton
import QtQuick 2.9
import Qt.labs.settings 1.0

Settings {
    property bool lazyLoad: true
    property bool richText: true
    property bool pressAndHold
    property bool rearrangeByActivity

    property bool darkTheme
    property bool miniMode
}
