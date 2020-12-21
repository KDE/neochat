/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.neochat 1.0

import NeoChat.Component 1.0

import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: i18n("Login")

    header: QQC2.Control {
        padding: Kirigami.Units.smallSpacing
        contentItem: Kirigami.InlineMessage {
            id: inlineMessage
            visible: false
            showCloseButton: true
        }
    }

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
        RowLayout {
            QQC2.Button {
                visible: Controller.accountCount > 0
                text: i18n("Cancel")
                onClicked: {
                    pageStack.layers.clear();
                }
            }
            QQC2.Button {
                text: i18n("Login")
                onClicked: doLogin()
            }
        }

        Connections {
            target: Controller
            function onErrorOccured(error, detail) {
                inlineMessage.type = Kirigami.MessageType.Error;
                if (detail && detail.length !== 0) {
                    inlineMessage.text = i18n("%1: %2", error, detail);
                } else {
                    inlineMessage.text = error;
                }
                inlineMessage.visible = true;
            }
        }
    }

    function doLogin() {
        inlineMessage.text = i18n("Loading, this might take up to 10 seconds.");
        inlineMessage.type = Kirigami.MessageType.Information
        inlineMessage.visible = true;
        if (accessTokenField.text.length > 0) {
            Controller.loginWithAccessToken(serverField.text.trim(), usernameField.text.trim(), accessTokenField.text, deviceNameField.text.trim());
        } else {
            Controller.loginWithCredentials(serverField.text.trim(), usernameField.text.trim(), passwordField.text, deviceNameField.text.trim());
        }
    }
}
