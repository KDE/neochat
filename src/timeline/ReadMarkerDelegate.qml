// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

TimelineDelegate {
    id: root
    contentItem: QQC2.ItemDelegate {
        padding: Kirigami.Units.largeSpacing
        topInset: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.largeSpacing * 2

        property bool isTemporaryHighlighted: false

        onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) {
            temporaryHighlightTimer.start();
        }

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
                    return Kirigami.Theme.positiveBackgroundColor;
                } else {
                    return Kirigami.Theme.backgroundColor;
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
                ColorAnimation {
                    target: readMarkerBackground
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
