// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: root

    property string toolTip
    property bool showTones: false

    QQC2.ToolTip.text: root.toolTip
    QQC2.ToolTip.visible: hovered && root.toolTip !== ""
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    readonly property color __pressedColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.3)
    readonly property color __hoverSelectColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.2)
    readonly property color __checkedBorderColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.7)
    readonly property color __pressedBorderColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.9)

    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false

        implicitHeight: Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2

        color: "transparent"

        Rectangle {
            width: parent.width - Kirigami.Units.largeSpacing
            height: parent.height - Kirigami.Units.largeSpacing
            anchors.centerIn: parent

            radius: Kirigami.Units.cornerRadius
            color: root.down ? root.__pressedColor : (root.checked || root.hovered ? root.__hoverSelectColor : "transparent")

            border.color: root.checked ? root.__checkedBorderColor : (root.down ? root.__pressedBorderColor : color)
            border.width: 1

            Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
            Behavior on border.color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
        }
    }

    contentItem: Item {
        Kirigami.Heading {
            anchors.fill: parent
            text: root.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "emoji"

            Kirigami.Icon {
                width: Kirigami.Units.gridUnit * 0.5
                height: Kirigami.Units.gridUnit * 0.5
                source: "arrow-down"
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                visible: root.showTones
            }
        }
    }
}
