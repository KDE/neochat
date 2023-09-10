// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatConnection connection

    FormCard.FormHeader {
        title: i18n("Server Capabilities")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Can change password")
            description: root.connection.canChangePassword
        }
    }
    FormCard.FormHeader {
        title: i18n("Default Room Version")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: root.connection.defaultRoomVersion
        }
    }
    FormCard.FormHeader {
        title: i18n("Available Room Versions")
    }
    FormCard.FormCard {
        Repeater {
            model: room.connection.getSupportedRoomVersions()

            delegate: FormCard.FormTextDelegate {
                text: modelData.id
                contentItem.children: QQC2.Label {
                    text: modelData.status
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }
    }
}
