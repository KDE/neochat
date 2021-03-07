/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0

import QtWebView 1.15

Kirigami.ScrollablePage {
    title: i18n("Register")
    
    Kirigami.FormLayout {
        Controls.TextField {
            id: homeserver
            
            Kirigami.FormData.label: i18n("Homeserver:")
            text: "https://kde.modular.im"
        }
        Controls.TextField {
            id: username
            
            Kirigami.FormData.label: i18n("Username:")
            text: "neochatregistrationtest1"
        }

        Controls.TextField {
            id: email

            Kirigami.FormData.label: i18n("E-mail address (optional):")            
        }
        Controls.TextField {
            id: password
            Kirigami.FormData.label: i18n("Password:")
            echoMode: TextInput.Password
            text: "asdfasdfasdfasdfasdf"
        }
        Controls.TextField {
            id: confirmPassword
            Kirigami.FormData.label: i18n("Confirm password:")
            echoMode: TextInput.Password
            text: "asdfasdfasdfasdfasdf"
        }
        Controls.Button {
            text: i18n("Register")
            enabled: homeserver.text !== "" && username.text !== "" && password.text !== "" && password.text === confirmPassword.text //TODO: check homeserver url, username, password if acceptable
            onClicked: Controller.flows(homeserver.text)
        }
    }
}
