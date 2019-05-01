import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Setting 0.1

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: "Enter Font Family"

    contentItem: AutoTextField {
        Layout.fillWidth: true

        id:fontFamilyField

        text: MSettings.fontFamily
        placeholderText: "Font Family"
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: MSettings.fontFamily = fontFamilyField.text

    onClosed: destroy()
}
