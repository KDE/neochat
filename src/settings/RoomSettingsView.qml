// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma Singleton

import QtQuick
import org.kde.kirigamiaddons.settings as KirigamiSettings

import org.kde.neochat

KirigamiSettings.ConfigurationView {
    id: root

    enum Type {
        Room,
        Space
    }

    property NeoChatRoom _room
    property NeoChatConnection _connection

    function openRoomSettings(room: NeoChatRoom, type: int): void {
        root._room = room;
        root._connection = room.connection;
        if (type === RoomSettingsView.Type.Space) {
            root.title = i18nc("@title:window", "Space Settings");
        } else {
            root.title = i18nc("@title:window", "Room Settings");
        }
        open();
    }

    objectName: "settingsPage"
    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18nc("@title", "General")
            icon.name: "settings-configure"
            page: () => Qt.createComponent("org.kde.neochat.settings", "RoomGeneralPage")
            initialProperties: () => {
                return {
                    room: root._room,
                    connection: root._connection
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "security"
            text: i18nc("@title", "Security")
            icon.name: "security-low"
            page: () => Qt.createComponent("org.kde.neochat.settings", "RoomSecurityPage")
            initialProperties: () => {
                return {
                    room: root._room
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "members"
            text: i18nc("@title", "Members")
            icon.name: "system-users-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "Members")
            initialProperties: () => {
                return {
                    room: root._room
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "permissions"
            text: i18nc("@title", "Permissions")
            icon.name: "visibility"
            page: () => Qt.createComponent("org.kde.neochat.settings", "Permissions")
            initialProperties: () => {
                return {
                    room: root._room
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "notifications"
            text: i18nc("@title", "Notifications")
            icon.name: "notifications"
            page: () => Qt.createComponent("org.kde.neochat.settings", "PushNotification")
            initialProperties: () => {
                return {
                    room: root._room
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "profile"
            text: i18nc("@title", "Profile")
            icon.name: "user-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "RoomProfile")
            initialProperties: () => {
                return {
                    room: root._room
                };
            }
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "advanced"
            text: i18nc("@title", "Advanced")
            icon.name: "document-properties-symbolic"
            page: () => Qt.createComponent("org.kde.neochat.settings", "RoomAdvancedPage")
            initialProperties: () => {
                return {
                    room: root._room
                };
            }
        }
    ]
}
