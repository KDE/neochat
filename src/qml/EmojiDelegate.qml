// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: root

    property string name
    property string emoji
    property bool showTones: false
    property bool isImage: false

    QQC2.ToolTip.text: root.name
    QQC2.ToolTip.visible: hovered && root.name !== ""
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    leftInset: Kirigami.Units.smallSpacing
    topInset: Kirigami.Units.smallSpacing
    rightInset: Kirigami.Units.smallSpacing
    bottomInset: Kirigami.Units.smallSpacing

    contentItem: Item {
        Kirigami.Heading {
            anchors.fill: parent
            visible: !root.emoji.startsWith("image") && !root.isImage
            text: root.emoji
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
        Image {
            anchors.fill: parent
            visible: root.emoji.startsWith("image") || root.isImage
            source: visible ? root.emoji : ""
        }
    }

    background: Rectangle {
        color: root.checked ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing

        Rectangle {
            radius: Kirigami.Units.smallSpacing
            anchors.fill: parent
            color: Kirigami.Theme.highlightColor
            opacity: root.hovered && !root.pressed ? 0.2 : 0
        }
    }
}
