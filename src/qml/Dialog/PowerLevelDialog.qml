// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.OverlaySheet {
    id: root
    title: i18n("Edit user power level")

    property NeoChatRoom room
    property var userId
    property int powerLevel

    onSheetOpenChanged: {
        if (sheetOpen) {
            powerLevelComboBox.currentIndex = powerLevelComboBox.indexOfValue(root.powerLevel)
        }
    }
    Kirigami.FormLayout {
        QQC2.ComboBox {
            id: powerLevelComboBox
            model: ListModel {
                id: powerLevelModel
            }
            textRole: "text"
            valueRole: "powerLevel"
            popup.z: root.z + 1 // Otherwise the popup will be behind the overlay sheet.

            // Done this way so we can have translated strings.
            Component.onCompleted: {
                powerLevelModel.append({"text":  i18n("Member (0)"), "powerLevel": 0});
                powerLevelModel.append({"text":  i18n("Moderator (50)"), "powerLevel": 50});
                powerLevelModel.append({"text":  i18n("Admin (100)"), "powerLevel": 100});
            }
        }
        QQC2.Button {
            text: i18n("Confirm")
            onClicked: {
                room.setUserPowerLevel(root.userId, powerLevelComboBox.currentValue)
                root.close()
                root.destroy()
            }
        }
    }
}
