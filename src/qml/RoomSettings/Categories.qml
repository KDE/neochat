// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.18 as Kirigami
import QtQuick.Layouts 1.15

Kirigami.CategorizedSettings {
    id: root
    property var room

    objectName: "settingsPage"
    actions: [
        Kirigami.SettingAction {
            actionName: "general"
            text: i18n("General")
            icon.name: "settings-configure"
            page: Qt.resolvedUrl("General.qml")
            initialProperties: {
                return {
                    room: root.room
                }
            }
        },
        Kirigami.SettingAction {
            actionName: "security"
            text: i18n("Security")
            icon.name: "security-low"
            page: Qt.resolvedUrl("Security.qml")
            initialProperties: {
                return {
                    room: root.room
                }
            }
        },
        Kirigami.SettingAction {
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
        Kirigami.SettingAction {
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
