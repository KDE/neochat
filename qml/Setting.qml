import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

Page {
    SwipeView {
        id: settingView

        currentIndex: 1
        anchors.fill: parent

        Item {
            id: accountPage
        }
        Item {
            id: secondPage
        }
        Item {
            id: thirdPage
        }
    }


    header: TabBar {
        id: tabBar
        width: parent.width
        currentIndex: settingView.currentIndex

        TabButton {
            text: qsTr("Account")
        }
        TabButton {
            text: qsTr("Call History")
        }
        TabButton {
            text: qsTr("Dail Pad")
        }
    }
}
