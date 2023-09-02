// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) usernameField.forceActiveFocus()

    FormCard.FormTextFieldDelegate {
        id: usernameField
        label: i18n("Username:")
        placeholderText: "user"
        onTextChanged: timer.restart()
        statusMessage: Registration.status === Registration.UsernameTaken ? i18n("Username unavailable") : ""
        Keys.onReturnPressed: {
            if (root.nextAction.enabled) {
                root.nextAction.trigger()
            }
        }
    }

    Timer {
        id: timer
        interval: 500
        onTriggered: Registration.username = usernameField.text
    }

    nextAction: Kirigami.Action {
        text: Registration.status === Registration.TestingUsername ? i18n("Loading") : null

        onTriggered: root.processed("qrc:/RegisterPassword.qml")
        enabled: Registration.status === Registration.Ready
    }

    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/Homeserver.qml")
    }
}
