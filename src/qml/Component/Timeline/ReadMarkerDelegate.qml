// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import Qt.labs.qmlmodels 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineDelegate {
    id: root
    contentItem: QQC2.ItemDelegate {
        padding: Kirigami.Units.largeSpacing
        topInset: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.largeSpacing * 2

        property bool isTemporaryHighlighted: false

        onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) temporaryHighlightTimer.start()

        Timer {
            id: temporaryHighlightTimer

            interval: 1500
            onTriggered: isTemporaryHighlighted = false
        }

        contentItem: QQC2.Label {
            text: i18nc("Relative time since the room was last read", "Last read: %1", time)
        }

        background: Kirigami.ShadowedRectangle {
            id: readMarkerBackground
            color: {
                if (root.isTemporaryHighlighted) {
                    return Kirigami.Theme.positiveBackgroundColor
                } else {
                    return Kirigami.Theme.backgroundColor
                }
            }
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            opacity: root.isTemporaryHighlighted ? 1 : 0.6
            radius: Kirigami.Units.smallSpacing
            shadow.size: Kirigami.Units.smallSpacing
            shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
            border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
            border.width: 1

            Behavior on color {
                ColorAnimation {target: readMarkerBackground; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.InOutCubic}
            }
        }
    }
}
