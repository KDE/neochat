/* SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Templates 2.15
import org.kde.kirigami 2.14 as Kirigami

Item {
    id: root
    property alias target: root.parent

    Rectangle {
        id: cursorLine
        property real previousX: 0
        property real previousY: 0
        parent: target
        implicitWidth: target.cursorRectangle.width
        implicitHeight: target.cursorRectangle.height
        x: Math.floor(target.cursorRectangle.x)
        y: Math.floor(target.cursorRectangle.y)

        color: target.color
        SequentialAnimation {
            id: blinkAnimation
            running: root.visible && Qt.styleHints.cursorFlashTime != 0 && target.selectionStart === target.selectionEnd
            PropertyAction {
                target: cursorLine
                property: "opacity"
                value: 1
            }
            PauseAnimation {
                duration: Qt.styleHints.cursorFlashTime/2
            }
            SequentialAnimation {
                loops: Animation.Infinite
                OpacityAnimator {
                    target: cursorLine
                    from: 1
                    to: 0
                    duration: Qt.styleHints.cursorFlashTime/2
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    target: cursorLine
                    from: 0
                    to: 1
                    duration: Qt.styleHints.cursorFlashTime/2
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    Connections {
        target: root.target
        function onCursorPositionChanged() {
            blinkAnimation.restart()
        }
    }
}


