// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat

QQC2.TextArea {
    id: root

    required property int style

    property bool highlight: false

    property bool sizeText: true

    leftPadding: lineRow.visible ? lineRow.width + lineRow.anchors.leftMargin + Kirigami.Units.smallSpacing : Kirigami.Units.largeSpacing
    verticalAlignment: Text.AlignVCenter

    readOnly: true
    selectByMouse: false

    RowLayout {
        id: lineRow
        anchors {
            top: root.top
            bottom: root.bottom
            left: root.left
            leftMargin: Kirigami.Units.smallSpacing
        }

        visible: root.style === LibNeoChat.RichFormat.Code

        QQC2.Label {
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            text: "1"
            color: Kirigami.Theme.disabledTextColor

            font.family: "monospace"
        }
        Kirigami.Separator {
            Layout.fillHeight: true
        }
    }

    StyleDelegateHelper {
        textItem: root
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Theme.colorSet: root.style === LibNeoChat.RichFormat.Quote ? Kirigami.Theme.Window : Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        radius: Kirigami.Units.cornerRadius
        border {
            width: 1
            color: root.highlight ?
            Kirigami.Theme.highlightColor :
            Kirigami.ColorUtils.linearInterpolation(
                Kirigami.Theme.backgroundColor,
                Kirigami.Theme.textColor,
                Kirigami.Theme.frameContrast
            )
        }
    }
}
