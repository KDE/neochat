import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: "Start a Chat"

    contentItem: ColumnLayout {
        AutoTextField {
            Layout.fillWidth: true

            id: identifierField

            placeholderText: "Room Alias/User ID"
        }
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: {
        var identifier = identifierField.text
        var firstChar = identifier.charAt(0)
        if (firstChar == "@") {
            spectralController.createDirectChat(spectralController.connection, identifier)
        } else if (firstChar == "!" || firstChar == "#") {
            spectralController.joinRoom(spectralController.connection, identifier)
        }
    }

    onClosed: destroy()
}
