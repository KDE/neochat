// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

ColumnLayout {
    MobileForm.FormCard {
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormCardHeader {
                title: i18n("Server Capabilities")
            }
            MobileForm.FormTextDelegate {
                text: i18n("Can change password")
                description: Controller.activeConnection.canChangePassword
            }
        }
    }
    MobileForm.FormCard {
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormCardHeader {
                title: i18n("Default Room Version")
            }
            MobileForm.FormTextDelegate {
                text: Controller.activeConnection.defaultRoomVersion
            }
        }
    }
    MobileForm.FormCard {
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormCardHeader {
                title: i18n("Available Room Versions")
            }
            Repeater {
                model: Controller.getSupportedRoomVersions(room.connection)

                delegate: MobileForm.FormTextDelegate {
                    text: modelData.id
                    contentItem.children: QQC2.Label {
                        text: modelData.status
                        color: Kirigami.Theme.disabledTextColor
                    }
                }
            }
        }
    }
    Item {
        Layout.fillHeight: true
    }
}
