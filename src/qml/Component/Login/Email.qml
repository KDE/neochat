// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) emailField.forceActiveFocus()

    FormCard.FormTextFieldDelegate {
        id: emailField
        label: i18n("Add an e-mail address:")
        placeholderText: "user@example.com"
        onTextChanged: Registration.email = text
        Keys.onReturnPressed: {
            if (root.nextAction.enabled) {
                root.nextAction.trigger()
            }
        }
    }

    FormCard.FormTextDelegate {
        id: confirmMessage
        text: i18n("Confirm e-mail address")
        visible: false
        description: i18n("A confirmation e-mail has been sent to your address. Please continue here <b>after</b> clicking on the confirmation link in the e-mail")
    }

    FormCard.FormButtonDelegate {
        id: resendButton
        text: i18nc("@button", "Re-send confirmation e-mail")
        onClicked: Registration.registerEmail()
        visible: false
    }

    nextAction: Kirigami.Action {
        enabled: emailField.text.length > 0
        onTriggered: {
            if (confirmMessage.visible) {
                Registration.registerAccount()
            } else {
                Registration.registerEmail()
                confirmMessage.visible = true
                resendButton.visible = true
            }
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/Username.qml")
    }
}
