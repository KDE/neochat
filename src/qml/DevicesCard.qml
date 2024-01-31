// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

ColumnLayout {
    id: root

    required property string title
    required property var type
    required property bool showVerifyButton

    visible: deviceRepeater.count > 0

    FormCard.FormHeader {
        title: root.title
    }

    FormCard.FormCard {
        id: devicesCard

        Repeater {
            id: deviceRepeater
            model: DevicesProxyModel {
                sourceModel: devicesModel
                type: root.type
            }

            Kirigami.LoadingPlaceholder {
                visible: deviceModel.count === 0 // We can assume 0 means loading since there is at least one device
                anchors.centerIn: parent
            }

            delegate: DeviceDelegate {
                showVerifyButton: root.showVerifyButton
            }
        }
    }
}
