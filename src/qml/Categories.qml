// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.settings as KirigamiSettings
import QtQuick.Layouts

import org.kde.neochat

KirigamiSettings.CategorizedSettings {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection

    objectName: "settingsPage"
    actions: [
        KirigamiSettings.SettingAction {
            actionName: "general"
            text: i18n("General")
            icon.name: "settings-configure"
            page: Qt.resolvedUrl("General.qml")
            initialProperties: {
                return {
                    room: root.room,
                    connection: root.connection
                }
            }
        },
        KirigamiSettings.SettingAction {
            actionName: "security"
            text: i18n("Security")
            icon.name: "security-low"
            page: Qt.resolvedUrl("RoomSecurity.qml")
            initialProperties: {
                return {
                    room: root.room
                }
            }
        },
        KirigamiSettings.SettingAction {
            actionName: "permissions"
            text: i18n("Permissions")
            icon.name: "visibility"
            page: Qt.resolvedUrl("Permissions.qml")
            initialProperties: {
                return {
                    room: root.room
                }
            }
        },
        KirigamiSettings.SettingAction {
            actionName: "notifications"
            text: i18n("Notifications")
            icon.name: "notifications"
            page: Qt.resolvedUrl("PushNotification.qml")
            initialProperties: {
                return {
                    room: root.room
                }
            }
        }
    ]
}
