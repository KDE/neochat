// SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

/**
 * @brief Line segment drawn on the left in the journey section view.
 *
 * Can be used in a fully drawn form, or partially drawn as a progress
 * overlay over the full variant.
 */
Item {
    id: root

    /**
     * @brief Whether the segment is representing the start of the journey.
     *
     * I.e. this will show a segment rounded at the top with a stop marker.
     */
    property bool isDeparture: false

    /**
     * @brief Whether the segment is representing the end of the journey.
     *
     * I.e. this will show a segment rounded at the bottom with a stop marker.
     */
    property bool isArrival: false

    /**
     * @brief The color of the segment.
     */
    property color lineColor: Kirigami.Theme.textColor

    /**
     * @brief The width of the segment.
     */
    property int lineWidth: Kirigami.Units.smallSpacing *4

    implicitWidth: root.lineWidth * 2
    clip: true

    Kirigami.ShadowedRectangle {
        id: line
        x: root.lineWidth / 2
        y: isDeparture? parent.height-height:0
        width: root.lineWidth
        color: root.lineColor

        corners {
            topRightRadius: isDeparture ? Math.round(width / 2) : 0
            topLeftRadius: isDeparture ? Math.round(width / 2) : 0

            bottomRightRadius: isArrival ? Math.round(width / 2) : 0
            bottomLeftRadius: isArrival ? Math.round(width / 2) : 0
        }
        height:
            if (isArrival) {
                Math.round(parent.height / 2) + root.lineWidth / 2
            } else if (isDeparture) {
                Math.round(parent.height / 2) + root.lineWidth / 2
            } else {
                parent.height
            }
    }

    Rectangle {
        id: stopDot
        x: line.x + (line.width - width) / 2
        y: parent.height / 2 - width / 2
        radius: width / 2
        width: root.lineWidth * 0.6
        height: width
        color: Kirigami.Theme.backgroundColor
        visible: root.isArrival || root.isDeparture
    }
}
