import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0

Dialog {
    property var room

    anchors.centerIn: parent
    width: 360

    id: root

    title: "Invite User"

    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    contentItem: AutoTextField {
        id: inviteUserDialogTextField
        placeholderText: "User ID"
    }

    onAccepted: room.inviteToRoom(inviteUserDialogTextField.text)

    onClosed: destroy()
}
