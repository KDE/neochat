// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.18 as Kirigami
import QtQuick.Layouts 1.15

Kirigami.CategorizedSettings {
    objectName: "settingsPage"
    actions: [
        Kirigami.SettingAction {
            text: i18n("General")
            icon.name: "org.kde.neochat"
            page: Qt.resolvedUrl("GeneralSettingsPage.qml")
        },
        Kirigami.SettingAction {
            text: i18n("Appearance")
            icon.name: "preferences-desktop-theme-global"
            page: Qt.resolvedUrl("AppearanceSettingsPage.qml")
        },
        Kirigami.SettingAction {
            text: i18n("Accounts")
            icon.name: "preferences-system-users"
            page: Qt.resolvedUrl("AccountsPage.qml")
        },
        Kirigami.SettingAction {
            text: i18n("Custom Emojis")
            icon.name: "preferences-desktop-emoticons"
            page: Qt.resolvedUrl("Emoticons.qml")
        },
        Kirigami.SettingAction {
            text: i18n("Spell Checking")
            iconName: "tools-check-spelling"
            page: Qt.resolvedUrl("SonnetConfigPage.qml")
        },
        Kirigami.SettingAction {
            text: i18n("Network Proxy")
            iconName: "network-connect"
            page: Qt.resolvedUrl("NetworkProxyPage.qml")
        },
        Kirigami.SettingAction {
            text: i18n("Devices")
            iconName: "computer"
            page: Qt.resolvedUrl("DevicesPage.qml")
        },
        Kirigami.SettingAction {
            text: i18n("About NeoChat")
            icon.name: "help-about"
            page: Qt.resolvedUrl("About.qml")
        }
    ]
}
