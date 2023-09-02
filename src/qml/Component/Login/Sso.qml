// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

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
            processed("qrc:/Loading.qml")
        }
    }

    FormCard.FormTextDelegate {
        text: i18n("Continue the login process in your browser.")
    }

    previousAction: Kirigami.Action {
        onTriggered: processed("qrc:/Login.qml")
    }

    nextAction: Kirigami.Action {
        text: i18nc("@action:button", "Re-open SSO URL")
        onTriggered: UrlHelper.openUrl(LoginHelper.ssoUrl)
    }
}

