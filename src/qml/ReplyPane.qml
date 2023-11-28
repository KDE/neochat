// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

RowLayout {
    id: root

    property string userName
    property color userColor
    property url userAvatar: ""
    property var text

    signal cancel

    Rectangle {
        id: verticalBorder

        Layout.fillHeight: true

        implicitWidth: Kirigami.Units.smallSpacing
        color: userColor
    }
    ColumnLayout {
        RowLayout {
            KirigamiComponents.Avatar {
                id: replyAvatar

                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small

                source: userAvatar
                name: userName
                color: userColor
            }
            QQC2.Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft

                color: userColor
                text: userName
                elide: Text.ElideRight
            }
        }
        QQC2.TextArea {
            id: textArea

            Layout.fillWidth: true

            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            text: "<style> a{color:" + Kirigami.Theme.linkColor + ";}.user-pill{}</style>" + replyTextMetrics.elidedText
            selectByMouse: true
            selectByKeyboard: true
            readOnly: true
            wrapMode: TextEdit.Wrap
            textFormat: TextEdit.RichText
            background: Item {}
            HoverHandler {
                cursorShape: textArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }

            TextMetrics {
                id: replyTextMetrics

                text: root.text
                font: textArea.font
                elide: Qt.ElideRight
                elideWidth: textArea.width * 2 - Kirigami.Units.smallSpacing * 2
            }
        }
    }
    QQC2.ToolButton {
        id: cancelButton

        Layout.alignment: Qt.AlignVCenter

        display: QQC2.AbstractButton.IconOnly
        text: i18nc("@action:button", "Cancel reply")
        icon.name: "dialog-close"
        onClicked: {
            root.cancel()
        }
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
