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

    onActiveFocusChanged: if (activeFocus) matrixIdField.forceActiveFocus()

    Component.onCompleted: {
        LoginHelper.matrixId = ""
    }

    FormCard.FormTextFieldDelegate {
        id: matrixIdField
        label: i18n("Matrix ID:")
        placeholderText: "@user:example.org"
        Accessible.name: i18n("Matrix ID")
        onTextChanged: {
            LoginHelper.matrixId = text
        }

        Keys.onReturnPressed: {
            root.nextAction.trigger()
        }
    }

    nextAction: Kirigami.Action {
        text: LoginHelper.isLoggedIn ? i18n("Already logged in") : (LoginHelper.testing && matrixIdField.acceptableInput) ?  i18n("Loading…") : i18nc("@action:button", "Continue")
        onTriggered: {
            if (LoginHelper.supportsSso && LoginHelper.supportsPassword) {
                processed("qrc:/org/kde/neochat/qml/LoginMethod.qml");
            } else if (LoginHelper.supportsSso) {
                processed("qrc:/org/kde/neochat/qml/Sso.qml");
            } else {
                processed("qrc:/org/kde/neochat/qml/Password.qml");
            }
        }
        enabled: LoginHelper.homeserverReachable
    }
    previousAction: Kirigami.Action {
        onTriggered: {
            root.processed("qrc:/org/kde/neochat/qml/LoginRegister.qml")
        }
    }
}
