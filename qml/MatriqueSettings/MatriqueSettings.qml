pragma Singleton
import QtQuick 2.9
import Qt.labs.settings 1.0

Settings {
    property bool lazyLoad: true
    property bool asyncMessageDelegate
    property bool richText
    property bool pressAndHold

    property bool darkTheme
    property bool miniMode
}
