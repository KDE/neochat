// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    noControls: true

    Component.onCompleted: LoginHelper.loginWithSso()

    Connections {
        target: LoginHelper
        function onSsoUrlChanged() {
            UrlHelper.openUrl(LoginHelper.ssoUrl)
        }
        function onConnected() {
            processed("qrc:/org/kde/neochat/qml/Loading.qml")
        }
    }

    FormCard.FormTextDelegate {
        text: i18n("Continue the login process in your browser.")
    }

    previousAction: Kirigami.Action {
        onTriggered: processed("qrc:/org/kde/neochat/qml/Login.qml")
    }

    nextAction: Kirigami.Action {
        text: i18nc("@action:button", "Re-open SSO URL")
        onTriggered: UrlHelper.openUrl(LoginHelper.ssoUrl)
    }
}

