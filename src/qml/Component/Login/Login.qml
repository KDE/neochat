// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

LoginStep {
    id: login

    showContinueButton: true
    showBackButton: false

    title: i18nc("@title", "Login")
    message: i18n("Enter your Matrix ID")

    Component.onCompleted: {
        LoginHelper.matrixId = ""
    }

    Kirigami.FormLayout {
        QQC2.TextField {
            id: matrixIdField
            Kirigami.FormData.label: i18n("Matrix ID:")
            placeholderText: "@user:matrix.org"
            onTextChanged: {
                if (acceptableInput) {
                    LoginHelper.matrixId = text
                }
            }

            Component.onCompleted: {
                matrixIdField.forceActiveFocus()
            }

            Keys.onReturnPressed: {
                login.action.trigger()
            }

            validator: RegularExpressionValidator {
                regularExpression: /^\@?[a-zA-Z0-9\._=\-/]+\:[a-zA-Z0-9\-]+(\.[a-zA-Z0-9\-]+)*(\:[0-9]+)?$/
            }
        }
    }

    action: Kirigami.Action {
        text: LoginHelper.testing && matrixIdField.acceptableInput ? (LoginHelper.isLoggedIn ? i18n("Already logged in") : i18n("Loading…")) : i18nc("@action:button", "Continue")
        onTriggered: {
            if (LoginHelper.supportsSso && LoginHelper.supportsPassword) {
                processed("qrc:/LoginMethod.qml");
            } else if (LoginHelper.supportsSso) {
                processed("qrc:/Sso.qml");
            } else {
                processed("qrc:/Password.qml");
            }
        }
        enabled: LoginHelper.homeserverReachable
    }
}
