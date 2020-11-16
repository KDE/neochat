/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
    property var room

    anchors.centerIn: parent
    width: 360

    id: root

    title: i18n("Invitation Received")
    modal: true

    contentItem: Label {
        text: i18n("Accept this invitation?")
    }

    footer: DialogButtonBox {
        Button {
            text: i18n("Accept")
            flat: true

            onClicked: {
                room.acceptInvitation()
                close()
            }
        }

        Button {
            text: i18n("Reject")
            flat: true

            onClicked: {
                room.forget()
                close()
            }
        }

        Button {
            text: i18n("Cancel")
            flat: true

            onClicked: close()
        }
    }

    onClosed: destroy()
}

