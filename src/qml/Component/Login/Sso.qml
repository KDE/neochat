// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0

LoginStep {
    id: root

    title: i18nc("@title", "Login")
    message: i18n("Login with single sign-on")

    Kirigami.FormLayout {
        Connections {
            target: LoginHelper
            function onSsoUrlChanged() {
                UrlHelper.openUrl(LoginHelper.ssoUrl)
                root.showMessage(i18n("Complete the authentication steps in your browser"))
                loginButton.enabled = true
                loginButton.text = i18n("Login")
            }
            function onConnected() {
                processed("qrc:/Loading.qml")
            }
        }
        RowLayout {
            QQC2.Button {
                text: i18nc("@action:button", "Back")

                onClicked: {
                    module.source = "qrc:/Login.qml"
                }
            }
            QQC2.Button {
                id: loginButton
                text: i18n("Login")
                onClicked: {
                    LoginHelper.loginWithSso()
                    loginButton.enabled = false
                    loginButton.text = i18n("Loading…")
                }
                Component.onCompleted: forceActiveFocus()
                Keys.onReturnPressed: clicked()
            }
        }
    }
}
