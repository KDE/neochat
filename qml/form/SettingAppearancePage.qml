import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    property alias darkTheme: themeSwitch.checked
    property alias miniMode: miniModeSwitch.checked

    Column {
        Switch {
            id: themeSwitch
            text: "Dark theme"
        }

        Switch {
            id: miniModeSwitch
            text: "Mini Room List"
        }
    }
}
