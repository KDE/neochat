// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

Flow {
    id: root

    property var avatarSize: Kirigami.Units.iconSizes.small
    property alias model: avatarFlowRepeater.model
    property string toolTipText
    property alias excessAvatars: excessAvatarsLabel.text

    spacing: -avatarSize / 2
    Repeater {
        id: avatarFlowRepeater
        delegate: KirigamiComponents.Avatar {
            required property var modelData

            implicitWidth: root.avatarSize
            implicitHeight: root.avatarSize

            name: modelData.displayName
            source: modelData.avatarSource
            color: modelData.color
        }
    }
    QQC2.Label {
        id: excessAvatarsLabel
        visible: text !== ""
        color: Kirigami.Theme.textColor
        horizontalAlignment: Text.AlignHCenter
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

    QQC2.ToolTip.text: toolTipText
    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    HoverHandler {
        id: hoverHandler
        margin: Kirigami.Units.smallSpacing
    }
}
