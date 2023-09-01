// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

ColumnLayout {
    FormCard.FormHeader {
        title: i18n("Server Capabilities")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Can change password")
            description: Controller.activeConnection.canChangePassword
        }
    }
    FormCard.FormHeader {
        title: i18n("Default Room Version")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: Controller.activeConnection.defaultRoomVersion
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
