// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

QQC2.Dialog {
    id: root
    title: i18nc("@title", "Edit User Power Level")

    property NeoChatRoom room
    property var userId
    property int powerLevel

    width: Kirigami.Units.gridUnit * 24
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: QQC2.Dialog.Ok
    onAccepted: {
        root.room.setUserPowerLevel(root.userId, powerLevelComboBox.currentValue);
        root.close();
        root.destroy();
    }

    onOpened: {
        if (root.opened) {
            powerLevelComboBox.currentIndex = powerLevelComboBox.indexOfValue(root.powerLevel);
        }
    }

    ColumnLayout {
        FormCard.FormComboBoxDelegate {
            id: powerLevelComboBox

            text: i18n("New power level")
            model: PowerLevelModel {}
            textRole: "name"
            valueRole: "value"
        }
    }
}
