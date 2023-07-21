// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Templates 2.15 as T
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.delegates 1.0 as Delegates
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

Delegates.RoundedItemDelegate {
    id: root

    required property url source

    signal contextMenuRequested()

    padding: Kirigami.Units.largeSpacing

    QQC2.ToolTip.visible: hovered
    QQC2.ToolTip.text: text
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    onPressAndHold: root.contextMenuRequested()

    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse
        onTapped: root.contextMenuRequested()
    }

    contentItem: KirigamiComponents.Avatar {
        source: root.source
        name: root.text
    }
}