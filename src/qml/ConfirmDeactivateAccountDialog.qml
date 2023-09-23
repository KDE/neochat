// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Deactivate Account")

    FormCard.FormHeader {
        title: i18nc("@title", "Deactivate Account")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18nc("@title", "Warning")
            description: i18n("Your account will be permanently disabled.\nThis cannot be undone.\nYour Matrix ID will not be available for new accounts.\nYour messages will stay available.")
        }

        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18n("Password")
            echoMode: TextInput.Password
        }

        FormCard.FormButtonDelegate {
            text: i18n("Deactivate account")
            icon.name: "emblem-warning"
            enabled: passwordField.text.length > 0
            onClicked: {
                root.connection.deactivateAccount(passwordField.text)
                root.closeDialog()
            }
        }
    }
}
