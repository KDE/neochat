// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

ColumnLayout {
    id: root

    required property string title
    required property var type
    required property bool showVerifyButton

    visible: deviceRepeater.count > 0
    MobileForm.FormHeader {
        title: root.title
        Layout.fillWidth: true
    }

    MobileForm.FormCard {
        id: devicesCard

        Layout.fillWidth: true


        contentItem: ColumnLayout {
            spacing: 0

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
}


