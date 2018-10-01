import QtQuick 2.9

import Spectral.Setting 0.1

MouseArea {
    signal primaryClicked()
    signal secondaryClicked()

    acceptedButtons: MSettings.pressAndHold ? Qt.LeftButton : (Qt.LeftButton | Qt.RightButton)

    onClicked: mouse.button == Qt.RightButton ? secondaryClicked() : primaryClicked()
    onPressAndHold: MSettings.pressAndHold ? secondaryClicked() : {}
}
