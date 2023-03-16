// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15

import org.kde.kirigami 2.18 as Kirigami
import org.kde.quickcharts 1.0 as Charts

/**
 * @brief A circular progress bar that fills an arc as progress goes up.
 */
Rectangle {
    id: root

    /**
     * @brief Progress of the circle as a percentage.
     *
     * Range - 0% to 100%.
     */
    property int progress: 0

    /**
     * @brief Offset angle for the start of the pie fill arc.
     *
     * This defaults to 0, i.e. an upward vertical line from the center. This rotates
     * that start point by the desired number of degrees.
     *
     * Range - 0 degrees to 360 degrees
     */
    property int startOffset: 0

    /**
     * @brief Fill color of the pie.
     */
    property color pieColor: Kirigami.Theme.highlightColor

    width: Kirigami.Units.iconSizes.smallMedium
    height: Kirigami.Units.iconSizes.smallMedium
    radius: width / 2
    color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.15)

    Charts.PieChart {
        id: chart
        anchors.fill: parent
        anchors.margins: 1

        filled: true
        // Set chart background color so the parent filled rectangle looks like
        // an outline.
        backgroundColor: Kirigami.Theme.backgroundColor
        fromAngle: root.startOffset
        toAngle: 360 + root.startOffset
        range {
            from: 0
            to: 100
            automatic: false
        }
        valueSources: Charts.SingleValueSource {
            value: root.progress
        }
        colorSource: Charts.SingleValueSource {
            value: Kirigami.Theme.highlightColor
        }
    }
}
