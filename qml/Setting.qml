import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

Page {
    property alias darkTheme: themeSwitch.checked
    property alias miniMode: miniModeSwitch.checked

    header: TabBar {
        id: tabBar
        width: parent.width
        currentIndex: settingView.currentIndex

        TabButton {
            text: qsTr("Account")
        }
        TabButton {
            text: qsTr("Appearance")
        }
        TabButton {
            text: qsTr("About")
        }
    }

    SwipeView {
        id: settingView

        currentIndex: tabBar.currentIndex
        anchors.fill: parent

        Item {
            id: accountPage
        }

        Item {
            id: appearancePage

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

        Item {
            id: thirdPage
        }
    }
}
