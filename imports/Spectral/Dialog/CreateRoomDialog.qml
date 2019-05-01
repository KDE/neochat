import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: "Create a Room"

    contentItem: ColumnLayout {
        AutoTextField {
            Layout.fillWidth: true

            id: roomNameField

            placeholderText: "Room Name"
        }

        AutoTextField {
            Layout.fillWidth: true

            id: roomTopicField

            placeholderText: "Room Topic"
        }
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: spectralController.createRoom(spectralController.connection, roomNameField.text, roomTopicField.text)

    onClosed: destroy()
}
