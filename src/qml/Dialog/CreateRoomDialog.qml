// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15

import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Create a Room")

    required property NeoChatConnection connection

    Component.onCompleted: roomNameField.forceActiveFocus()

    FormCard.FormHeader {
        title: i18nc("@title", "Room Information")
    }
    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: roomNameField
            label: i18n("Room name:")
            onAccepted: if (roomNameField.text.length > 0) roomTopicField.forceActiveFocus();
        }

        FormCard.FormTextFieldDelegate {
            id: roomTopicField
            label: i18n("Room topic:")
            onAccepted: ok.clicked()
        }

        FormCard.FormButtonDelegate {
            id: ok
            text: i18nc("@action:button", "Ok")
            enabled: roomNameField.text.length > 0
            onClicked: {
                root.connection.createRoom(roomNameField.text, roomTopicField.text);
                root.closeDialog()
            }
        }
    }
}
