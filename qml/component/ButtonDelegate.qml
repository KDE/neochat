import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3

Item {
    property int index
    property alias contentItem: itemDelegate.contentItem
    signal clicked

    id: buttonDelegate

    Layout.fillWidth: true
    Layout.preferredHeight: width

    Rectangle {
        width: swipeView.currentIndex === index ?  parent.width : 0
        height: parent.height
        anchors.bottom:  itemDelegate.bottom
        color: Qt.lighter(Material.accent)

        Behavior on width {
            PropertyAnimation { easing.type: Easing.InOutQuad; duration: 200 }
        }
    }

    ItemDelegate {
        id: itemDelegate
        anchors.fill: parent

        onClicked: {
            swipeView.currentIndex = index
            buttonDelegate.clicked()
        }
    }
}
