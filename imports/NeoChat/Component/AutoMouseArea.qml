import QtQuick 2.12

import NeoChat.Setting 0.1

MouseArea {
    signal primaryClicked()
    signal secondaryClicked()

    acceptedButtons: Qt.LeftButton | Qt.RightButton

    onClicked: mouse.button == Qt.RightButton ? secondaryClicked() : primaryClicked()
    onPressAndHold: secondaryClicked()
}
