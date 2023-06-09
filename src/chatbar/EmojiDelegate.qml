// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Button {
    id: root

    required property string toolTip
    property bool showTones: false

    QQC2.ToolTip.text: toolTip
    QQC2.ToolTip.visible: hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    flat: true

    contentItem: Kirigami.Heading {
        text: root.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Kirigami.Icon {
            width: Kirigami.Units.gridUnit * 0.5
            height: Kirigami.Units.gridUnit * 0.5
            source: "arrow-down-symbolic"
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            visible: root.showTones
        }
    }
}
