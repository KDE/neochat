// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.18 as Kirigami
import QtQuick.Layouts 1.15

Kirigami.CategorizedSettings {
    objectName: "settingsPage"
    actions: [
        Kirigami.SettingAction {
            actionName: "general"
            text: i18n("General")
            icon.name: "org.kde.neochat"
            page: Qt.resolvedUrl("GeneralSettingsPage.qml")
        },
        Kirigami.SettingAction {
            actionName: "appearance"
            text: i18n("Appearance")
            icon.name: "preferences-desktop-theme-global"
            page: Qt.resolvedUrl("AppearanceSettingsPage.qml")
        },
        Kirigami.SettingAction {
            actionName: "notifications"
            text: i18n("Notifications")
            icon.name: "preferences-desktop-notification"
            page: Qt.resolvedUrl("GlobalNotificationsPage.qml")
        },
        Kirigami.SettingAction {
            actionName: "accounts"
            text: i18n("Accounts")
            icon.name: "preferences-system-users"
            page: Qt.resolvedUrl("AccountsPage.qml")
        },
        Kirigami.SettingAction {
            actionName: "customEmojis"
            text: i18n("Custom Emojis")
            icon.name: "preferences-desktop-emoticons"
            page: Qt.resolvedUrl("Emoticons.qml")
        },
        Kirigami.SettingAction {
            actionName: "spellChecking"
            text: i18n("Spell Checking")
            icon.name: "tools-check-spelling"
            page: Qt.resolvedUrl("SonnetConfigPage.qml")
            visible: Qt.platform.os !== "android"
        },
        Kirigami.SettingAction {
            actionName: "netowrkProxy"
            text: i18n("Network Proxy")
            icon.name: "network-connect"
            page: Qt.resolvedUrl("NetworkProxyPage.qml")
        },
        Kirigami.SettingAction {
            actionName: "devices"
            text: i18n("Devices")
            icon.name: "computer"
            page: Qt.resolvedUrl("DevicesPage.qml")
        },
        Kirigami.SettingAction {
            actionName: "about"
            text: i18n("About NeoChat")
            icon.name: "help-about"
            page: Qt.resolvedUrl("About.qml")
        },
        Kirigami.SettingAction {
            text: i18n("About KDE")
            icon.name: "kde"
            page: Qt.resolvedUrl("AboutKDE.qml")
        }
    ]
}
