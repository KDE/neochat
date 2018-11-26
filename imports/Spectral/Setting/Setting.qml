pragma Singleton
import QtQuick 2.9
import Qt.labs.settings 1.0

Settings {
    property bool showNotification: true

    property bool pressAndHold
    property bool showTray: true

    property bool darkTheme

    property bool enableTimelineBackground: true
    property string timelineBackground
}
