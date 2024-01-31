// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) {
        passwordField.forceActiveFocus();
    }

    FormCard.FormTextFieldDelegate {
        id: passwordField
        label: i18n("Password:")
        echoMode: TextInput.Password
        onTextChanged: Registration.password = text
        Keys.onReturnPressed: {
            confirmPasswordField.forceActiveFocus();
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
                root.nextAction.trigger();
            }
        }
    }

    nextAction: Kirigami.Action {
        onTriggered: {
            passwordField.enabled = false;
            Registration.registerAccount();
        }
        enabled: passwordField.text === confirmPasswordField.text
    }

    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/org/kde/neochat/qml/Username.qml")
    }
}
