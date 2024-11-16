// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma Singleton

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings
import QtQuick.Layouts

import org.kde.neochat

KirigamiSettings.ConfigurationView {
    id: root

    property NeoChatConnection connection

    objectName: "settingsPage"
    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18n("General")
            icon.name: "org.kde.neochat"
            page: () => Qt.createComponent("org.kde.neochat.settings", "NeoChatGeneralPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "appearance"
            text: i18n("Appearance")
            icon.name: "preferences-desktop-theme-global"
            page: () => Qt.createComponent("org.kde.neochat.settings", "AppearanceSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "notifications"
            text: i18n("Notifications")
            icon.name: "preferences-desktop-notification"
            page: () => Qt.createComponent("org.kde.neochat.settings", "GlobalNotificationsPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "security"
            text: i18nc("@title", "Security & Safety")
            icon.name: "preferences-security"
            page: () => Qt.createComponent("org.kde.neochat.settings", "NeoChatSecurityPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "accounts"
            text: i18n("Accounts")
            icon.name: "preferences-system-users"
            page: () => Qt.createComponent("org.kde.neochat.settings", "AccountsPage")
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "emoticons"
            text: i18n("Stickers & Emojis")
            icon.name: "preferences-desktop-emoticons"
            page: () => Qt.createComponent("org.kde.neochat.settings", "EmoticonsPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
            visible: root.connection !== null
        },
        KirigamiSettings.SpellcheckingConfigurationModule {},
        KirigamiSettings.ConfigurationModule {
            moduleId: "networkProxy"
            text: i18n("Network Proxy")
            icon.name: "network-connect"
            page: () => Qt.createComponent("org.kde.neochat.settings", "NetworkProxyPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "devices"
            text: i18n("Devices")
            icon.name: "computer"
            page: () => Qt.createComponent("org.kde.neochat.settings", "DevicesPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutNeochat"
            text: i18n("About NeoChat")
            icon.name: "help-about"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
            category: i18nc("@title:group", "About")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutKDE"
            text: i18n("About KDE")
            icon.name: "kde"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage")
            category: i18nc("@title:group", "About")
        }
    ]
}
