// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat

QQC2.AbstractButton {
    id: root

    required property int style

    property bool open: false

    property bool compressed: false

    readonly property real uncompressedWidth: styleDelegate.implicitWidth + arrowIcon.implicitWidth + 1 + contentRow.spacing * 2 + padding * 2

    padding: Kirigami.Units.smallSpacing

    icon {
        width: Kirigami.Units.iconSizes.smallMedium
        height: Kirigami.Units.iconSizes.smallMedium
    }

    contentItem: RowLayout {
        id: contentRow
        StyleDelegate {
            id: styleDelegate
            Layout.fillWidth: true

            visible: !root.compressed
            style: root.style
            sizeText: false

            onPressed: root.clicked()
        }
        Kirigami.Icon {
            id: styleIcon
            visible: root.compressed
            source: root.icon.name
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
        }
        Kirigami.Separator {
            Layout.fillHeight: true
        }
        Kirigami.Icon {
            id: arrowIcon
            source: root.open ? "arrow-down" : "arrow-up"
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
        }
    }

    QQC2.ToolTip.text: text
    QQC2.ToolTip.visible: hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        radius: Kirigami.Units.cornerRadius
        border {
            width: root.hovered || root.open ? 1 : 0
            color: Kirigami.Theme.highlightColor
        }
    }
}
