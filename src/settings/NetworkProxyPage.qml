// SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-KDE-Accepted-LGPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Proxy")

    property int currentType
    property bool proxyConfigChanged: false

    FormCard.FormHeader {
        title: i18n("Network Proxy")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18n("System Default")
            checked: currentType === 0
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 0;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18n("No Proxy")
            checked: currentType === 3
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 3;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18n("HTTP")
            checked: currentType === 1
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 1;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18n("Socks5")
            checked: currentType === 2
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 2;
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Proxy Settings")
    }
    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: hostField
            label: i18n("Host")
            text: NeoChatConfig.proxyHost
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true;
            }
        }
        FormCard.FormSpinBoxDelegate {
            id: portField
            label: i18n("Port")
            value: NeoChatConfig.proxyPort
            from: 0
            to: 65536
            textFromValue: function (value, locale) {
                return value; // it will add a thousands separator if we don't do this, not sure why
            }
            onValueChanged: {
                proxyConfigChanged = true;
            }
        }
        FormCard.FormTextFieldDelegate {
            id: userField
            label: i18n("User")
            text: NeoChatConfig.proxyUser
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true;
            }
        }
        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18n("Password")
            text: NeoChatConfig.proxyPassword
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true;
            }
        }
    }

    footer: QQC2.ToolBar {
        height: visible ? implicitHeight : 0
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                text: i18n("Apply")
                enabled: currentType !== NeoChatConfig.proxyType || proxyConfigChanged
                onClicked: {
                    NeoChatConfig.proxyType = currentType;
                    NeoChatConfig.proxyHost = hostField.text;
                    NeoChatConfig.proxyPort = portField.value;
                    NeoChatConfig.proxyUser = userField.text;
                    NeoChatConfig.proxyPassword = passwordField.text;
                    NeoChatConfig.save();
                    proxyConfigChanged = false;
                    ProxyController.setApplicationProxy();
                }
            }
        }
    }

    Component.onCompleted: {
        currentType = NeoChatConfig.proxyType;
    }
}
