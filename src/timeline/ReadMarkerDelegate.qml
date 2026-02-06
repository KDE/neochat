// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

import org.kde.neochat

TimelineDelegate {
    id: root

    /**
     * @brief The timestamp of the event as a neoChatDateTime.
     */
    required property neoChatDateTime dateTime

    property bool isTemporaryHighlighted: false
    onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) {
        temporaryHighlightTimer.start();
    }

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: QQC2.ItemDelegate {
        padding: Kirigami.Units.largeSpacing
        topInset: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.largeSpacing * 2
        bottomPadding: Kirigami.Units.largeSpacing * 2
        leftPadding: Kirigami.Units.largeSpacing * 2
        bottomInset: Kirigami.Units.largeSpacing

        Timer {
            id: temporaryHighlightTimer

            interval: 1500
            onTriggered: root.isTemporaryHighlighted = false
        }

        contentItem: QQC2.Label {
            text: i18nc("Relative time since the room was last read", "Last read: %1", root.dateTime.relativeDateTime)
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
            radius: Kirigami.Units.cornerRadius
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
