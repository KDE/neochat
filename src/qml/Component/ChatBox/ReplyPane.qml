// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.14 as Kirigami

import org.kde.neochat 1.0

GridLayout {
    id: root
    property string userName
    property color userColor: Kirigami.Theme.highlightColor
    property var userAvatar: ""
    property bool isReply
    property var text

    rows: 3
    columns: 3
    rowSpacing: Kirigami.Units.smallSpacing
    columnSpacing: Kirigami.Units.largeSpacing

    QQC2.Label {
        id: replyLabel
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft
        Layout.columnSpan: 3
        topPadding: Kirigami.Units.smallSpacing

        text: isReply ? i18n("Replying to:") : i18n("Editing message:")
    }
    Rectangle {
        id: verticalBorder

        Layout.fillHeight: true
        Layout.rowSpan: 2

        implicitWidth: Kirigami.Units.smallSpacing
        color: userColor
    }
    Kirigami.Avatar {
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
    QQC2.TextArea {
        id: textArea

        Layout.fillWidth: true
        Layout.columnSpan: 2

        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0
        text: "<style> a{color:" + Kirigami.Theme.linkColor + ";}.user-pill{}</style>" + replyTextMetrics.elidedText
        selectByMouse: true
        selectByKeyboard: true
        readOnly: true
        wrapMode: QQC2.Label.Wrap
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
