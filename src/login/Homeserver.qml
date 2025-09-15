// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) {
        urlField.forceActiveFocus();
    }

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
                root.nextAction.trigger();
            }
        }
    }

    Timer {
        id: timer
        interval: 500
        onTriggered: Registration.homeserver = urlField.text
    }

    Connections {
        target: Registration
        function onConnected(connection): void {
            root.processed("Loading");
        }
    }

    nextAction: Kirigami.Action {
        text: Registration.testing ? i18n("Loading") : Registration.status === Registration.Oidc ? i18nc("@action:button", "Continue in Browser") : null
        enabled: Registration.status > Registration.ServerNoRegistration
        onTriggered: {
            if (Registration.status === Registration.Oidc) {
                Qt.openUrlExternally(Registration.oidcUrl)
            } else {
                root.processed("Username")
            }
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("LoginRegister")
    }
}
