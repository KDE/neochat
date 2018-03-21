import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3

Page {
    property alias theme: themeSwitch.checked
    header: TabBar {
        id: settingBar
        width: parent.width
        z: 10
        currentIndex: settingBar.currentIndex

        TabButton {
            text: qsTr("Overview")
        }

        TabButton {
            text: qsTr("Interface")
        }

        TabButton {
            text: qsTr("Network")
        }

        TabButton {
            text: qsTr("Sync")
        }
    }

    SwipeView {
        id: settingSwipe

        currentIndex: settingBar.currentIndex
        anchors.fill: parent

        Page {

        }

        Page {
            Column {
                width: parent.width
                Switch {
                    id: themeSwitch
                    text: qsTr("Dark Theme")
                }
            }
        }

        Page {

        }

        Page {

        }
    }
}
