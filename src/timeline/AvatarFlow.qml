// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

RowLayout {
    id: root

    property var avatarSize: Kirigami.Units.iconSizes.small
    property alias model: root.limiterModel.sourceModel
    property string toolTipText

    property LimiterModel limiterModel: LimiterModel {
        maximumCount: 5
    }

    spacing: -avatarSize / 2
    Repeater {
        id: avatarFlowRepeater
        model: root.limiterModel

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

        Layout.preferredWidth: Math.max(excessAvatarsTextMetrics.advanceWidth + Kirigami.Units.largeSpacing * 2, height)
        Layout.preferredHeight: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing
        Layout.fillHeight: true

        visible: root.limiterModel.extraCount > 0
        color: Kirigami.Theme.textColor
        horizontalAlignment: Text.AlignHCenter

        text: "+ " + root.limiterModel.extraCount

        background: Kirigami.ShadowedRectangle {
            color: Kirigami.Theme.backgroundColor
            radius: Math.ceil(height / 2)
            shadow {
                size: Kirigami.Units.smallSpacing
                color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
            }
            border {
                color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                width: 1
            }
        }

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
