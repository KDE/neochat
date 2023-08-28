// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.12 as QQC2

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) passwordField.forceActiveFocus()

    FormCard.FormTextFieldDelegate {
        id: passwordField
        label: i18n("Password:")
        echoMode: TextInput.Password
        onTextChanged: Registration.password = text
        Keys.onReturnPressed: {
            confirmPasswordField.forceActiveFocus()
        }
    }

    FormCard.FormTextFieldDelegate {
        id: confirmPasswordField
        label: i18n("Confirm Password:")
        enabled: passwordField.enabled
        echoMode: TextInput.Password
        statusMessage: passwordField.text.length === confirmPasswordField.text.length && passwordField.text !== confirmPasswordField.text ? i18n("The passwords do not match.") : ""
        Keys.onReturnPressed: {
            if (root.nextAction.enabled) {
                root.nextAction.trigger()
            }
        }
    }

    nextAction: Kirigami.Action {
        onTriggered: {
            passwordField.enabled = false
            Registration.registerAccount()
        }
        enabled: passwordField.text === confirmPasswordField.text
    }

    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/Username.qml")
    }
}
