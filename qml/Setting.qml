import QtQuick 2.10
import QtQuick.Controls 2.3

Page {
    TabBar {
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

        }

        Page {

        }

        Page {

        }
    }
}
