// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

// Not to be confused with the Shimmer project.
// I like their gradiented GTK themes though.

import QtQuick
import org.kde.kirigami as Kirigami

Gradient {
    id: root

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
            target: root
            properties: "pos"
        }
        PauseAnimation {
            duration: 300
        }
    }

    GradientStop { position: root.pos-root.offset; color: root.translucent }
    GradientStop { position: root.pos; color: root.bright }
    GradientStop { position: root.pos+root.offset; color: root.translucent }
}
