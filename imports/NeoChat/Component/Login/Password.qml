// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

LoginStep {
    id: password

    title: i18nc("@title", "Password")
    message: i18n("Enter your password")
    showContinueButton: true
    showBackButton: true
    previousUrl: LoginHelper.isLoggingIn ? "" : LoginHelper.supportsSso ? "qrc:/imports/NeoChat/Component/Login/LoginMethod.qml" : "qrc:/imports/NeoChat/Component/Login/Login.qml"

    action: Kirigami.Action {
        text: i18nc("@action:button", "Login")
        enabled: passwordField.text.length > 0 && !LoginHelper.isLoggingIn
        onTriggered: {
            LoginHelper.login();
        }
        iconName: "go-next"
    }

    Connections {
        target: LoginHelper
        function onConnected() {
            processed("qrc:/imports/NeoChat/Component/Login/Loading.qml")
        }
    }

    Kirigami.FormLayout {
        Kirigami.PasswordField {
            id: passwordField
            onTextChanged: LoginHelper.password = text
            enabled: !LoginHelper.isLoggingIn

            Component.onCompleted: {
                passwordField.forceActiveFocus()
            }

            Keys.onReturnPressed: {
                password.action.trigger()
            }
        }
    }
}
