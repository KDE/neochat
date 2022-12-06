// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

QQC2.ItemDelegate {
    id: emojiDelegate

    property string name
    property string emoji
    property bool showTones: false

    QQC2.ToolTip.text: emojiDelegate.name
    QQC2.ToolTip.visible: hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    leftInset: Kirigami.Units.smallSpacing
    topInset: Kirigami.Units.smallSpacing
    rightInset: Kirigami.Units.smallSpacing
    bottomInset: Kirigami.Units.smallSpacing

    contentItem: Item {
        Kirigami.Heading {
            anchors.fill: parent
            visible: !emojiDelegate.emoji.startsWith("image")
            text: emojiDelegate.emoji
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "emoji"

            Kirigami.Icon {
                width: Kirigami.Units.gridUnit * 0.5
                height: Kirigami.Units.gridUnit * 0.5
                source: "arrow-down"
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                visible: emojiDelegate.showTones
            }
        }
        Image {
            anchors.fill: parent
            visible: emojiDelegate.emoji.startsWith("image")
            source: visible ? emojiDelegate.emoji : ""
        }
    }

    background: Rectangle {
        color: emojiDelegate.checked ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing

        Rectangle {
            radius: Kirigami.Units.smallSpacing
            anchors.fill: parent
            color: Kirigami.Theme.highlightColor
            opacity: emojiDelegate.hovered && !emojiDelegate.pressed ? 0.2 : 0
        }
    }
}
