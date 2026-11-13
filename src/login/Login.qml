// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.ki18n

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus)
        matrixIdField.forceActiveFocus()

    Component.onCompleted: {
        LoginHelper.matrixId = "";
    }

    FormCard.FormTextFieldDelegate {
        id: matrixIdField
        label: KI18n.i18n("Matrix ID:")
        text: LoginHelper.matrixId
        placeholderText: "@user:example.org"
        Accessible.name: KI18n.i18n("Matrix ID")
        onTextChanged: {
            LoginHelper.matrixId = text;
        }

        Keys.onReturnPressed: {
            root.nextAction.trigger();
        }
    }

    nextAction: Kirigami.Action {
        text: LoginHelper.isLoggedIn ? KI18n.i18n("Already logged in") : (LoginHelper.testing && matrixIdField.acceptableInput) ? KI18n.i18n("Loading…") : KI18n.i18nc("@action:button", "Continue")
        onTriggered: {
            if (LoginHelper.supportsSso && LoginHelper.supportsPassword) {
                root.processed("LoginMethod");
            } else if (LoginHelper.supportsSso) {
                root.processed("Sso");
            } else {
                root.processed("Password");
            }
        }
        enabled: LoginHelper.homeserverReachable
    }
    previousAction: Kirigami.Action {
        onTriggered: {
            root.processed("LoginRegister");
        }
    }
}
