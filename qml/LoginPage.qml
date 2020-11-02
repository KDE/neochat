/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-or-later
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import Spectral.Component 2.0

import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: i18n("Login")

    required property var spectralController

    Kirigami.FormLayout {
        id: formLayout
        QQC2.TextField {
            id: serverField
            Kirigami.FormData.label: i18n("Server Address")
            text: "https://matrix.org"
            onAccepted: usernameField.forceActiveFocus()
        }
        QQC2.TextField {
            id: usernameField
            Kirigami.FormData.label: i18n("Username")
            onAccepted: passwordField.forceActiveFocus()
        }
        Kirigami.PasswordField {
            id: passwordField
            Kirigami.FormData.label: i18n("Password")
            onAccepted: accessTokenField.forceActiveFocus()
        }
        QQC2.TextField {
            id: accessTokenField
            Kirigami.FormData.label: i18n("Access Token (Optional)")
            onAccepted: deviceNameField.forceActiveFocus()
        }
        QQC2.TextField {
            id: deviceNameField
            Kirigami.FormData.label: i18n("Device Name (Optional)")
            onAccepted: doLogin()
        }
        QQC2.Button {
            text: i18n("Login")
            onClicked: doLogin()
        }
    }

    function doLogin() {
        if (accessTokenField.text.length > 0) {
            spectralController.loginWithAccessToken(serverField.text, usernameField.text, accessTokenField.text, deviceNameField.text)
        } else {
            spectralController.loginWithCredentials(serverField.text, usernameField.text, passwordField.text, deviceNameField.text)
        }
    }
}
