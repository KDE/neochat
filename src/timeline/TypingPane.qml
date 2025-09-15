/* SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Loader {
    id: root
    property string labelText: ""

    active: visible
    sourceComponent: QQC2.Pane {
        id: typingPane

        leftPadding: Kirigami.Units.largeSpacing
        rightPadding: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.largeSpacing

        FontMetrics {
            id: fontMetrics
        }

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        contentItem: RowLayout {
            spacing: typingPane.spacing
            Row {
                id: dotRow
                property int duration: 400
                spacing: Kirigami.Units.smallSpacing
                Repeater {
                    model: 3
                    delegate: Rectangle {
                        id: dot

                        required property int index

                        color: Kirigami.Theme.textColor
                        radius: height / 2
                        implicitWidth: fontMetrics.xHeight
                        implicitHeight: fontMetrics.xHeight
                        // rotating 45 degrees makes the dots look a bit smoother when scaled up
                        rotation: 45
                        opacity: 0.5
                        scale: 1
                        // FIXME: Sometimes the animation timings for each
                        // dot drift slightly reletative to each other.
                        // Not everyone can see this, but I'm pretty sure it's there.
                        SequentialAnimation {
                            running: true
                            PauseAnimation {
                                duration: dotRow.duration * dot.index / 2
                            }
                            SequentialAnimation {
                                loops: Animation.Infinite
                                ParallelAnimation {
                                    // Animators unfortunately sync up instead of being
                                    // staggered, so I'm using NumberAnimations instead.
                                    NumberAnimation {
                                        target: dot
                                        property: "scale"
                                        from: 1
                                        to: 1.33
                                        duration: dotRow.duration
                                    }
                                    NumberAnimation {
                                        target: dot
                                        property: "opacity"
                                        from: 0.5
                                        to: 1
                                        duration: dotRow.duration
                                    }
                                }
                                ParallelAnimation {
                                    NumberAnimation {
                                        target: dot
                                        property: "scale"
                                        from: 1.33
                                        to: 1
                                        duration: dotRow.duration
                                    }
                                    NumberAnimation {
                                        target: dot
                                        property: "opacity"
                                        from: 1
                                        to: 0.5
                                        duration: dotRow.duration
                                    }
                                }
                                PauseAnimation {
                                    duration: dotRow.duration
                                }
                            }
                        }
                    }
                }
            }
            QQC2.Label {
                id: typingLabel
                elide: Text.ElideRight
                text: root.labelText
                textFormat: Text.PlainText
            }
        }

        leftInset: !mirrored ? 0 : -(background as Rectangle).radius
        rightInset: mirrored ? 0 : -(background as Rectangle).radius
        bottomInset: -(background as Rectangle).radius
        background: Rectangle {
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor
            border.color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
            border.width: 1
        }
    }
}
