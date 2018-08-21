import QtQuick 2.9
import MatriqueSettings 0.1

MouseArea {
    signal primaryClicked()
    signal secondaryClicked()

    propagateComposedEvents: true
    acceptedButtons: MatriqueSettings.pressAndHold ? Qt.LeftButton : (Qt.LeftButton | Qt.RightButton)
    onClicked: mouse.button == Qt.RightButton ? secondaryClicked() : primaryClicked()
    onPressAndHold: MatriqueSettings.pressAndHold ? secondaryClicked() : {}
}
