// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.settings 1.0 as KirigamiSettings

import org.kde.neochat 1.0

KirigamiSettings.CategorizedSettings {
    id: root

    property NeoChatRoom room

    actions: [
        KirigamiSettings.SettingAction {
            actionName: "roomData"
            text: i18n("Room Data")
            icon.name: "datatype"
            page: "qrc:/RoomData.qml"
            initialProperties: {
                return {
                    room: root.room
                }
            }
        },
        KirigamiSettings.SettingAction {
            actionName: "serverData"
            text: i18n("Server Info")
            icon.name: "network-server-symbolic"
            page: "qrc:/ServerData.qml"
        }
    ]
}
