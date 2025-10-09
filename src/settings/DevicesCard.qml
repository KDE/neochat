// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

ColumnLayout {
    id: root

    default property alias delegates: devicesCard.delegates
    required property string title
    required property var type
    required property bool showVerifyButton
    required property DevicesModel devicesModel

    visible: deviceRepeater.count > 0

    FormCard.FormHeader {
        title: root.title
    }

    FormCard.FormCard {
        id: devicesCard

        Repeater {
            id: deviceRepeater
            model: DevicesProxyModel {
                sourceModel: root.devicesModel
                type: root.type
            }

            delegate: DeviceDelegate {
                showVerifyButton: root.showVerifyButton
                devicesModel: root.devicesModel
            }
        }
    }
}
