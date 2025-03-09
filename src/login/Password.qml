// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    property Homeserver homeserver

    Connections {
        target: LoginHelper
        function onConnected() {
            processed("Loading");
        }
    }

    onActiveFocusChanged: if (activeFocus)
        passwordField.forceActiveFocus()

    FormCard.FormTextFieldDelegate {
        id: passwordField

        label: i18n("Password:")
        enabled: !LoginHelper.isLoggingIn
        echoMode: TextInput.Password
        Accessible.name: i18n("Password")
        statusMessage: LoginHelper.isInvalidPassword ? i18n("Invalid username or password") : ""

        Keys.onReturnPressed: {
            root.nextAction.trigger();
        }
    }

    nextAction: Kirigami.Action {
        text: i18nc("@action:button", "Login")
        // enabled: passwordField.text.length > 0 && !LoginHelper.isLoggingIn
        onTriggered: {
            root.clearError();
            let pending = Accounts.loginWithPassword(root.homeserver.matrixId, passwordField.text)
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: processed("Login")
    }
}
