// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.18 as Kirigami
import org.kde.kirigamiaddons.settings 1.0 as KirigamiSettings
import QtQuick.Layouts 1.15

KirigamiSettings.CategorizedSettings {
    id: settingsPage

    required property var connection

    objectName: "settingsPage"
    actions: [
        KirigamiSettings.SettingAction {
            actionName: "general"
            text: i18n("General")
            icon.name: "org.kde.neochat"
            page: Qt.resolvedUrl("GeneralSettingsPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "appearance"
            text: i18n("Appearance")
            icon.name: "preferences-desktop-theme-global"
            page: Qt.resolvedUrl("AppearanceSettingsPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "notifications"
            text: i18n("Notifications")
            icon.name: "preferences-desktop-notification"
            page: Qt.resolvedUrl("GlobalNotificationsPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "accounts"
            text: i18n("Accounts")
            icon.name: "preferences-system-users"
            page: Qt.resolvedUrl("AccountsPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "emoticons"
            text: i18n("Stickers & Emojis")
            icon.name: "preferences-desktop-emoticons"
            page: Qt.resolvedUrl("EmoticonsPage.qml")
        },

        KirigamiSettings.SettingAction {
            actionName: "spellChecking"
            text: i18n("Spell Checking")
            icon.name: "tools-check-spelling"
            page: Qt.resolvedUrl("SonnetConfigPage.qml")
            visible: Qt.platform.os !== "android"
        },
        KirigamiSettings.SettingAction {
            actionName: "networkProxy"
            text: i18n("Network Proxy")
            icon.name: "network-connect"
            page: Qt.resolvedUrl("NetworkProxyPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "devices"
            text: i18n("Devices")
            icon.name: "computer"
            page: Qt.resolvedUrl("DevicesPage.qml")
            initialProperties: {
                return {
                    connection: settingsPage.connection
                }
            }
        },
        KirigamiSettings.SettingAction {
            actionName: "aboutNeochat"
            text: i18n("About NeoChat")
            icon.name: "help-about"
            page: Qt.resolvedUrl("About.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "aboutKDE"
            text: i18n("About KDE")
            icon.name: "kde"
            page: Qt.resolvedUrl("AboutKDE.qml")
        }
    ]
}
