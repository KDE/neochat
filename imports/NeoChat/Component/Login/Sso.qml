// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

LoginStep {
    id: root

    title: i18nc("@title", "Login")
    message: i18n("Login with single sign-on")

    Kirigami.FormLayout {
        Connections {
            target: LoginHelper
            function onSsoUrlChanged() {
                Qt.openUrlExternally(LoginHelper.ssoUrl)
            }
            function onConnected() {
                processed("qrc:/imports/NeoChat/Component/Login/Loading.qml")
            }
        }

        QQC2.Button {
            text: i18n("Login")
            onClicked: {
                LoginHelper.loginWithSso()
                root.showMessage(i18n("Complete the authentication steps in your browser"))
            }
            Component.onCompleted: forceActiveFocus()
            Keys.onReturnPressed: clicked()
        }
    }
}
