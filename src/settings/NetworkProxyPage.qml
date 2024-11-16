// SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    title: i18nc("@title:window", "Proxy")
    property int currentType
    property bool proxyConfigChanged: false

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormRadioDelegate {
            id: systemDefault
            text: i18n("System Default")
            checked: currentType === 0
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 0
            }
        }

        FormCard.FormDelegateSeparator { below: systemDefault; above: noProxy }

        FormCard.FormRadioDelegate {
            id:noProxy
            text: i18n("No Proxy")
            checked: currentType === 3
            enabled: !Config.isProxyTypeImmutable
            onToggled: {
                currentType = 3;
            }
        }

        FormCard.FormDelegateSeparator { below: noProxy; above: http }

        FormCard.FormRadioDelegate {
            id: http
            text: i18n("HTTP")
            checked: currentType === 1
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 1
            }
        }

        FormCard.FormDelegateSeparator { below: http; above: socks5 }

        FormCard.FormRadioDelegate {
            id: socks5
            text: i18n("Socks5")
            checked: currentType === 2
            enabled: !NeoChatConfig.isProxyTypeImmutable
            onToggled: {
                currentType = 2
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Proxy Settings")
    }

    FormCard.FormCard {
        // It makes no sense to configure proxy settings for "System Default" and "No Proxy"
        enabled: currentType !== 0 && currentType !== 3

        FormCard.FormTextFieldDelegate {
            id: hostField
            label: i18n("Host")
            text: NeoChatConfig.proxyHost
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true
            }
        }
        FormCard.FormDelegateSeparator { below: hostField; above: portField }
        // we probably still need a FormSpinBoxDelegate
        FormCard.AbstractFormDelegate {
            Layout.fillWidth: true
            contentItem: RowLayout {
                QQC2.Label {
                    text: i18n("Port")
                    Layout.fillWidth: true
                }
                QQC2.SpinBox {
                    id: portField
                    value: NeoChatConfig.proxyPort
                    from: 0
                    to: 65536
                    validator: IntValidator {bottom: portField.from; top: portField.to}
                    textFromValue: function(value, locale) {
                        return value // it will add a thousands separator if we don't do this, not sure why
                    }
                    onValueChanged: {
                        proxyConfigChanged = true
                    }
                }
            }
        }
        FormCard.FormDelegateSeparator { below: portField; above: userField }
        FormCard.FormTextFieldDelegate {
            id: userField
            label: i18n("User")
            text: NeoChatConfig.proxyUser
            inputMethodHints: Qt.ImhUrlCharactersOnly
            onEditingFinished: {
                proxyConfigChanged = true
            }
        }
        FormCard.FormDelegateSeparator { below: userField; above: passwordField }
        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18n("Password")
            text: NeoChatConfig.proxyPassword
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
                icon.name: "dialog-ok-apply-symbolic"
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
        proxyConfigChanged = false; // Make doubly sure that stupid bindings haven't turned this on
        currentType = NeoChatConfig.proxyType;
    }
}
