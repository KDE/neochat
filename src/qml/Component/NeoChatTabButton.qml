// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Templates 2.15 as T
import org.kde.kirigami 2.19 as Kirigami

import org.kde.neochat 1.0

T.TabButton {
    id: control

    property color foregroundColor: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.85)
    property color highlightForegroundColor: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.85)
    property color highlightBarColor: Kirigami.Theme.highlightColor

    property color pressedColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.3)
    property color hoverSelectColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.2)
    property color checkedBorderColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.7)
    property color pressedBorderColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.9)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    display: T.AbstractButton.TextUnderIcon

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    // not using the hover handler built into control, since it seems to misbehave and
    // permanently report hovered after a touch event
    HoverHandler {
        id: hoverHandler
    }

    padding: Kirigami.Units.smallSpacing
    spacing: Kirigami.Units.smallSpacing

    icon.height: control.display === T.AbstractButton.TextBesideIcon ? Kirigami.Units.iconSizes.small : Kirigami.Units.iconSizes.smallMedium
    icon.width: control.display === T.AbstractButton.TextBesideIcon ? Kirigami.Units.iconSizes.small : Kirigami.Units.iconSizes.smallMedium
    icon.color: control.checked ? control.highlightForegroundColor : control.foregroundColor

    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        Kirigami.Theme.inherit: false

        implicitHeight: Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2

        color: "transparent"

        Rectangle {
            width: parent.width - Kirigami.Units.largeSpacing
            height: parent.height - Kirigami.Units.largeSpacing
            anchors.centerIn: parent

            radius: Kirigami.Units.smallSpacing
            color: control.down ? pressedColor : (control.checked || hoverHandler.hovered ? hoverSelectColor : "transparent")

            border.color: control.checked ? checkedBorderColor : (control.down ? pressedBorderColor : color)
            border.width: 1

            Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
            Behavior on border.color { ColorAnimation { duration: Kirigami.Units.shortDuration } }
        }
    }

    contentItem: QQC2.Label {
        id: label
        Kirigami.MnemonicData.enabled: control.enabled && control.visible
        Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.MenuItem
        Kirigami.MnemonicData.label: control.text

        text: Kirigami.MnemonicData.richTextLabel
        horizontalAlignment: Text.AlignHCenter

        wrapMode: Text.Wrap
        elide: Text.ElideMiddle
        color: control.checked ? control.highlightForegroundColor : control.foregroundColor

        font.bold: control.checked
        font.family: "emoji"
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.20

        Behavior on color { ColorAnimation {} }
        Behavior on opacity { NumberAnimation {} }
    }
}
