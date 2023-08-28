// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) urlField.forceActiveFocus()

    FormCard.FormTextFieldDelegate {
        id: urlField
        label: i18n("Server Url:")
        validator: RegularExpressionValidator {
            regularExpression: /([a-zA-Z0-9\-]+\.)*[a-zA-Z0-9]+(:[0-9]+)?/
        }
        onTextChanged: timer.restart()
        statusMessage: Registration.status === Registration.ServerNoRegistration ? i18n("Registration is disabled on this server.") : ""
        Keys.onReturnPressed: {
            if (root.nextAction.enabled) {
                root.nextAction.trigger()
            }
        }
    }

    Timer {
        id: timer
        interval: 500
        onTriggered: Registration.homeserver = urlField.text
    }

    nextAction: Kirigami.Action {
        text: Registration.testing ? i18n("Loading") : null
        enabled: Registration.status > Registration.ServerNoRegistration
        onTriggered: root.processed("qrc:/Username.qml");
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/LoginRegister.qml")
    }
}
