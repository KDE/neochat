// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma Singleton

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings
import QtQuick.Layouts

import org.kde.neochat
import org.kde.ki18n

KirigamiSettings.ConfigurationView {
    id: root

    property NeoChatConnection connection

    objectName: "settingsPage"
    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: KI18n.i18n("General")
            icon.name: "org.kde.neochat.tray"
            page: () => Qt.createComponent("org.kde.neochat.settings", "NeoChatGeneralPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "appearance"
            text: KI18n.i18n("Appearance")
            icon.name: "preferences-desktop-theme-global-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "AppearanceSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "notifications"
            text: KI18n.i18n("Notifications")
            icon.name: "preferences-desktop-notification-symbolic"
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
            text: KI18n.i18nc("@title", "Security & Safety")
            icon.name: "preferences-security-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "NeoChatSecurityPage")
            initialProperties: () => {
                return {
                    connection: root.connection
                };
            }
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "devices"
            text: KI18n.i18n("Devices")
            icon.name: "computer-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "DevicesPage")
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            id: accountsModule
            moduleId: "accounts"
            text: KI18n.i18n("Accounts")
            icon.name: "preferences-system-users-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "AccountsPage")
            visible: root.connection !== null
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "emoticons"
            text: KI18n.i18n("Stickers & Emojis")
            icon.name: "preferences-desktop-emoticons-symbolic"
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
            text: KI18n.i18n("Network Proxy")
            icon.name: "network-connect-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "NetworkProxyPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "keyboardShortcuts"
            text: KI18n.i18n("Keyboard Shortcuts")
            icon.name: "input-keyboard-symbolic"
            page: () => Qt.createComponent("org.kde.kirigami.actioncollection", "ShortcutsEditor")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutNeochat"
            text: KI18n.i18n("About NeoChat")
            icon.name: "help-about-symbolic"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
            category: KI18n.i18nc("@title:group", "About")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutKDE"
            text: KI18n.i18n("About KDE")
            icon.name: "kde-symbolic"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage")
            category: KI18n.i18nc("@title:group", "About")
        }
    ]

    function openWithInitialProperties(defaultModule = '', initialProperties): void {
        let module = modules.find(module => module.moduleId == defaultModule) ?? null;
        if (module) {
            module.initialProperties = () => {
                return initialProperties;
            }
        }
        root.open(defaultModule);
    }
}
