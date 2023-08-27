// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root

    Connections {
        target: LoginHelper
        function onConnected() {
            processed("qrc:/Loading.qml")
        }
    }

    onActiveFocusChanged: if(activeFocus) passwordField.forceActiveFocus()

    FormCard.FormTextFieldDelegate {
        id: passwordField

        label: i18n("Password:")
        onTextChanged: LoginHelper.password = text
        enabled: !LoginHelper.isLoggingIn
        echoMode: TextInput.Password
        Accessible.name: i18n("Password")
        statusMessage: LoginHelper.isInvalidPassword ? i18n("Invalid username or password") : ""

        Keys.onReturnPressed: {
            root.nextAction.trigger()
        }
    }

    nextAction: Kirigami.Action {
        text: i18nc("@action:button", "Login")
        enabled: passwordField.text.length > 0 && !LoginHelper.isLoggingIn
        onTriggered: {
            root.clearError()
            LoginHelper.login();
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: processed("qrc:/Login.qml")
    }
}
