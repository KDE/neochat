// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus)
        matrixIdField.forceActiveFocus()

    property Homeserver homeserver

    Timer {
        id: timer
        interval: 500
        repeat: false
        onTriggered: if (matrixIdField.text.length > 0) {
            root.homeserver.resolveFromMatrixId(matrixIdField.text)
        }
    }

    FormCard.FormTextFieldDelegate {
        id: matrixIdField
        label: i18n("Matrix ID:")
        placeholderText: "@user:example.org"
        Accessible.name: i18n("Matrix ID")
        onTextChanged: {
            timer.restart()
        }

        Keys.onReturnPressed: {
            root.nextAction.trigger();
        }
    }

    nextAction: Kirigami.Action {
        // text: LoginHelper.isLoggedIn ? i18n("Already logged in") : (LoginHelper.testing && matrixIdField.acceptableInput) ? i18n("Loading…") : i18nc("@action:button", "Continue")
        onTriggered: {
            if (root.homeserver.ssoLoginSupported && root.homeserver.passwordLoginSupported) {
                processed("LoginMethod");
            } else if (root.homeserver.ssoLoginSupported) {
                processed("Sso");
            } else {
                processed("Password");
            }
        }
        enabled: root.homeserver.loginFlowsLoaded
    }
    previousAction: Kirigami.Action {
        onTriggered: {
            root.processed("LoginRegister");
        }
    }
}
