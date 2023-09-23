// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

Delegates.RoundedItemDelegate {
    id: root

    required property url source

    signal contextMenuRequested()
    signal selected()

    padding: Kirigami.Units.largeSpacing

    QQC2.ToolTip.visible: hovered
    QQC2.ToolTip.text: text
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    Accessible.onPressAction: selected();
    Keys.onSpacePressed: selected();
    Keys.onEnterPressed: selected();

    onPressAndHold: root.contextMenuRequested()

    TapHandler {
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onTapped: (eventPoint, button) => {
            if (button === Qt.RightButton) {
                root.contextMenuRequested();
            } else {
                root.selected();
            }
        }
    }

    contentItem: KirigamiComponents.Avatar {
        source: root.source
        name: root.text
    }
}
