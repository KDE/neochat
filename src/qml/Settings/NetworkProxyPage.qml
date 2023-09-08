// SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-KDE-Accepted-LGPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "General")

    property int currentType
    property bool proxyConfigChanged: false

    FormCard.FormHeader {
        title: i18n("Network Proxy")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18n("System Default")
            checked: currentType === 0
            enabled: !Config.isProxyTypeImmutable
            onToggled: {
                currentType = 0
            }
        }
        FormCard.FormRadioDelegate {
            text: i18n("HTTP")
            checked: currentType === 1
            enabled: !Config.isProxyTypeImmutable
            onToggled: {
                currentType = 1
            }
        }
        FormCard.FormRadioDelegate {
            text: i18n("Socks5")
            checked: currentType === 2
            enabled: !Config.isProxyTypeImmutable
            onToggled: {
                currentType = 2
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
            text: Config.proxyHost
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true
            }
        }
        FormCard.FormSpinBoxDelegate {
            id: portField
            label: i18n("Port")
            value: Config.proxyPort
            from: 0
            to: 65536
            textFromValue: function(value, locale) {
                return value // it will add a thousands separator if we don't do this, not sure why
            }
            onValueChanged: {
                proxyConfigChanged = true
            }
        }
        FormCard.FormTextFieldDelegate {
            id: userField
            label: i18n("User")
            text: Config.proxyUser
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true
            }
        }
        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18n("Password")
            text: Config.proxyPassword
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true
            }
        }
    }

    footer: QQC2.ToolBar {
        height: visible ? implicitHeight : 0
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button  {
                text: i18n("Apply")
                enabled: currentType !== Config.proxyType || proxyConfigChanged
                onClicked: {
                    Config.proxyType = currentType
                    Config.proxyHost = hostField.text
                    Config.proxyPort = portField.value
                    Config.proxyUser = userField.text
                    Config.proxyPassword = passwordField.text
                    Config.save()
                    proxyConfigChanged = false
                    Controller.setApplicationProxy()
                }
            }
        }
    }

    Component.onCompleted: {
        currentType = Config.proxyType
    }
}
