// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

// Not to be confused with the Shimmer project.
// I like their gradiented GTK themes though.

import QtQuick 2.15
import org.kde.kirigami 2.15 as Kirigami

Gradient {
    id: gradient

    orientation: Gradient.Horizontal

    property color color: Kirigami.Theme.textColor
    property color translucent: Qt.rgba(color.r, color.g, color.b, 0.2)
    property color bright: Qt.rgba(color.r, color.g, color.b, 0.3)
    property real pos: 0.5
    property real offset: 0.6

    property SequentialAnimation ani: SequentialAnimation {
        running: true
        loops: Animation.Infinite
        NumberAnimation {
            from: -2.0
            to: 2.0
            duration: 700
            target: gradient
            properties: "pos"
        }
        PauseAnimation {
            duration: 300
        }
    }

    GradientStop { position: gradient.pos-gradient.offset; color: gradient.translucent }
    GradientStop { position: gradient.pos; color: gradient.bright }
    GradientStop { position: gradient.pos+gradient.offset; color: gradient.translucent }
}