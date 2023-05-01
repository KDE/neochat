// SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-KDE-Accepted-LGPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "General")
    property int currentType
    property bool proxyConfigChanged: false

    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Network Proxy")
                }
                MobileForm.FormRadioDelegate {
                    text: i18n("System Default")
                    checked: currentType === 0
                    enabled: !Config.isProxyTypeImmutable
                    onToggled: {
                        currentType = 0
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18n("HTTP")
                    checked: currentType === 1
                    enabled: !Config.isProxyTypeImmutable
                    onToggled: {
                        currentType = 1
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18n("Socks5")
                    checked: currentType === 2
                    enabled: !Config.isProxyTypeImmutable
                    onToggled: {
                        currentType = 2
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Proxy Settings")
                }
                MobileForm.FormTextFieldDelegate {
                    id: hostField
                    label: i18n("Host")
                    text: Config.proxyHost
                    inputMethodHints: Qt.ImhUrlCharactersOnly
                    onEditingFinished: {
                        proxyConfigChanged = true
                    }
                }
                MobileForm.FormSpinBoxDelegate {
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
                MobileForm.FormTextFieldDelegate {
                    id: userField
                    label: i18n("User")
                    text: Config.proxyUser
                    inputMethodHints: Qt.ImhUrlCharactersOnly
                    onEditingFinished: {
                        proxyConfigChanged = true
                    }
                }
                MobileForm.FormTextFieldDelegate {
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
