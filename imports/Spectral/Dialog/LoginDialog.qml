import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: "Login"

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: doLogin()

    contentItem: ColumnLayout {
        AutoTextField {
            Layout.fillWidth: true

            id: serverField

            placeholderText: "Server Address"
            text: "https://matrix.org"
        }

        AutoTextField {
            Layout.fillWidth: true

            id: usernameField

            placeholderText: "Username"

            onAccepted: passwordField.forceActiveFocus()
        }

        AutoTextField {
            Layout.fillWidth: true

            id: passwordField

            placeholderText: "Password"
            echoMode: TextInput.Password

            onAccepted: root.doLogin()
        }
    }

    function doLogin() {
        spectralController.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)
    }

    onClosed: root.destroy()
}
