import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
    property var room

    anchors.centerIn: parent
    width: 360

    id: root

    title: "Invitation Received"
    modal: true

    contentItem: Label {
        text: "Accept this invitation?"
    }

    footer: DialogButtonBox {
        Button {
            text: "Accept"
            flat: true

            onClicked: {
                room.acceptInvitation()
                close()
            }
        }

        Button {
            text: "Reject"
            flat: true

            onClicked: {
                room.forget()
                close()
            }
        }

        Button {
            text: "Cancel"
            flat: true

            onClicked: close()
        }
    }

    onClosed: destroy()
}

