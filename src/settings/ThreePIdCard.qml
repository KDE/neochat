// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatConnection connection

    required property string title
    required property string medium

    visible: deviceRepeater.count > 0

    FormCard.FormHeader {
        title: root.title
    }
    FormCard.FormCard {
        id: devicesCard

        Repeater {
            id: deviceRepeater
            model: KSortFilterProxyModel {
                sourceModel: root.connection.threePIdModel
                filterRoleName: "medium"
                filterString: root.medium
            }

            delegate: FormCard.FormTextDelegate {
                required property string address
                text: address
            }
        }
    }
}
