// SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-KDE-Accepted-LGPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "General")
    property int currentType
    property bool proxyConfigChanged: false

    ColumnLayout {
        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.RadioButton {
                Kirigami.FormData.label: i18n("Network Proxy")
                text: i18n("System Default")
                checked: currentType === 0
                enabled: !Config.isProxyTypeImmutable
                onToggled: {
                    currentType = 0
                }
            }
            QQC2.RadioButton {
                text: i18n("HTTP")
                checked: currentType === 1
                enabled: !Config.isProxyTypeImmutable
                onToggled: {
                    currentType = 1
                }
            }
            QQC2.RadioButton {
                text: i18n("Socks5")
                checked: currentType === 2
                enabled: !Config.isProxyTypeImmutable
                onToggled: {
                    currentType = 2
                }
            }
            QQC2.TextField {
                id: hostField
                Kirigami.FormData.label: i18n("Proxy Host")
                text: Config.proxyHost
                inputMethodHints: Qt.ImhUrlCharactersOnly
                onEditingFinished: {
                    proxyConfigChanged = true
                }
            }
            QQC2.SpinBox {
                id: portField
                Kirigami.FormData.label: i18n("Proxy Port")
                value: Config.proxyPort
                from: 0
                to: 65536
                validator: IntValidator {bottom: from; top: to}
                textFromValue: function(value, locale) {
                    return value // it will add a thousands separator if we don't do this, not sure why
                }
                onValueChanged: {
                    proxyConfigChanged = true
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
