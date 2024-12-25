// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

Flow {
    id: root

    property var avatarSize: Kirigami.Units.iconSizes.small
    property alias model: avatarFlowRepeater.model
    property string toolTipText

    spacing: -avatarSize / 2
    Repeater {
        id: avatarFlowRepeater
        delegate: KirigamiComponents.Avatar {
            required property string displayName
            required property url avatarUrl
            required property color memberColor

            implicitWidth: root.avatarSize
            implicitHeight: root.avatarSize

            name: displayName
            source: avatarUrl
            color: memberColor
            asynchronous: true
        }
    }
    QQC2.Label {
        id: excessAvatarsLabel
        visible: text !== ""
        color: Kirigami.Theme.textColor
        horizontalAlignment: Text.AlignHCenter

        text: root.model?.excessReadMarkersString ?? ""

        background: Kirigami.ShadowedRectangle {
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            radius: height / 2
            shadow.size: Kirigami.Units.smallSpacing
            shadow.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
            border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
            border.width: 1
        }

        height: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing
        width: Math.max(excessAvatarsTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)

        TextMetrics {
            id: excessAvatarsTextMetrics
            text: excessAvatarsLabel.text
        }
    }

    QQC2.ToolTip.text: root.model?.readMarkersString ?? ""
    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    HoverHandler {
        id: hoverHandler
        margin: Kirigami.Units.smallSpacing
    }
}
