// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

Kirigami.Dialog {
    id: root
    title: i18nc("@title", "Edit User Power Level")

    property NeoChatRoom room
    property var userId
    property int powerLevel

    width: Kirigami.Units.gridUnit * 24

    standardButtons: QQC2.Dialog.NoButton

    onOpened: {
        if (root.opened) {
            powerLevelComboBox.currentIndex = powerLevelComboBox.indexOfValue(root.powerLevel)
        }
    }

    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: powerLevelComboBox

            text: i18n("New power level")
            model: ListModel {
                id: powerLevelModel
            }
            textRole: "text"
            valueRole: "powerLevel"

            // Done this way so we can have translated strings.
            Component.onCompleted: {
                powerLevelModel.append({"text": i18n("Member (0)"), "powerLevel": 0});
                powerLevelModel.append({"text": i18n("Moderator (50)"), "powerLevel": 50});
                powerLevelModel.append({"text": i18n("Admin (100)"), "powerLevel": 100});
            }
        }
    }
    customFooterActions: [
        Kirigami.Action {
            text: i18n("Confirm")
            icon.name: "dialog-ok"
            onTriggered: {
                root.room.setUserPowerLevel(root.userId, powerLevelComboBox.currentValue)
                root.close()
                root.destroy()
            }
        }
    ]
}
