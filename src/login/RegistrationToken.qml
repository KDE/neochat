// SPDX-FileCopyrightText: 2026 Zach Scott <dagg@kyzune.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) {
        registrationTokenField.forceActiveFocus();
    }

    FormCard.FormTextFieldDelegate {
        id: registrationTokenField
        label: i18nc("@label", "Registration Token:")
        echoMode: TextInput.Password
        onTextChanged: timer.restart()
        statusMessage: Registration.status === Registration.InvalidRegistrationToken ? i18nc("@info", "Invalid registration token") : ""
        Keys.onReturnPressed: {
            if (root.nextAction.enabled) {
                root.nextAction.trigger();
            }
        }
    }

    Timer {
        id: timer
        interval: 500
        onTriggered: Registration.registrationToken = registrationTokenField.text
    }

    nextAction: Kirigami.Action {
        onTriggered: Registration.registerAccount();
        enabled: Registration.status === Registration.Ready || Registration.status === Registration.NoRegistrationTokenPrevalidation
    }

    previousAction: Kirigami.Action {
        onTriggered: root.processed("RegisterPassword")
    }
}
