// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.neochat

QQC2.ItemDelegate {
    id: root

    property string name
    property string emoji
    property bool showTones: false
    property bool isImage: false

    QQC2.ToolTip.text: root.name
    QQC2.ToolTip.visible: hovered && root.name !== ""
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing
    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing

    contentItem: Item {
        QQC2.Label {
            anchors.fill: parent
            visible: !root.emoji.startsWith("mxc") && !root.isImage
            text: root.emoji
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "emoji"
            font.pixelSize: height - Kirigami.Units.largeSpacing
            elide: Text.ElideNone
            wrapMode: Text.NoWrap
            textFormat: Text.PlainText

            Kirigami.Icon {
                width: Kirigami.Units.gridUnit * 0.5
                height: Kirigami.Units.gridUnit * 0.5
                source: "arrow-down-symbolic"
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                visible: root.showTones
            }
        }
        Image {
            anchors.fill: parent
            visible: root.emoji.startsWith("mxc") || root.isImage
            source: visible ? root.emoji : ""
            fillMode: Image.PreserveAspectFit
            sourceSize.width: width
            sourceSize.height: height
        }
    }

    background: Rectangle {
        color: root.checked ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius

        Rectangle {
            radius: Kirigami.Units.cornerRadius
            anchors.fill: parent
            color: Kirigami.Theme.highlightColor
            opacity: root.hovered && !root.pressed ? 0.2 : 0
        }
    }
}
