import QtQuick 2.9
import QtQuick.Controls 2.2
import MatriqueSettings 0.1

Page {
    Column {
        Switch {
            id: themeSwitch
            text: "Dark theme"
            checked: MatriqueSettings.darkTheme
            onCheckedChanged: MatriqueSettings.darkTheme = checked
        }

        Switch {
            id: miniModeSwitch
            text: "Mini Room List"
            checked: MatriqueSettings.miniMode
            onCheckedChanged: MatriqueSettings.miniMode = checked
        }
    }
}
