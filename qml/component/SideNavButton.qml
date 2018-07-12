import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

Item {
    property var page
    property alias contentItem: buttonDelegate.contentItem
    signal clicked

    id: sideNavButton

    Layout.fillWidth: true
    Layout.preferredHeight: width

    Rectangle {
        width: stackView.currentItem === page ?  parent.width : 0
        height: parent.height
        anchors.bottom:  buttonDelegate.bottom
        color: Qt.lighter(Material.accent)

        Behavior on width {
            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
        }
    }

    ButtonDelegate {
        id: buttonDelegate
        anchors.fill: parent

        onClicked: {
            if(page && stackView.currentItem !== page) {
                if(stackView.depth === 1) {
                    stackView.replace(page)
                } else {
                    stackView.clear()
                    stackView.push(page)
                }
            }
            sideNavButton.clicked()
        }
    }
}
