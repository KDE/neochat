// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.labs.components as Components
import org.kde.kirigami as Kirigami
import org.kde.prison

Components.AbstractMaximizeComponent {
    id: root

    required property string text
    required property color avatarColor
    required property string avatarSource

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: root.close()
    }

    leading: Components.Avatar {
        id: userAvatar
        implicitWidth: Kirigami.Units.iconSizes.medium
        implicitHeight: Kirigami.Units.iconSizes.medium

        name: root.title
        source: root.avatarSource
        color: root.avatarColor
    }

    content: Item {
        Keys.onEscapePressed: root.close()
        Barcode {
            barcodeType: Barcode.QRCode
            content: root.text
            height: Math.min(parent.height, Kirigami.Units.gridUnit * 20)
            width: height
            anchors.centerIn: parent
        }
        MouseArea {
            id: closeArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onClicked: root.close()
        }
    }
}
