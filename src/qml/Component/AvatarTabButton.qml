// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Templates 2.15 as T
import org.kde.kirigami 2.19 as Kirigami

T.TabButton {
    id: control

    /**
     * @brief This property specifies the index of this tab within the tab bar.
     */
    readonly property int tabIndex: {
        let tabIdx = 0
        for (let i = 0; i < parent.children.length; ++i) {
            if (parent.children[i] === this) {
                return tabIdx
            }
            // Checking for AbstractButtons because any AbstractButton can act as a tab
            if (parent.children[i] instanceof T.AbstractButton) {
                ++tabIdx
            }
        }
        return -1
    }

    /**
     * @brief This property sets whether the icon colors should be masked with a single color.
     *
     * default: ``true``
     *
     * @since KDE Frameworks 5.96
     */
    property bool recolorIcon: true

    property color foregroundColor: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.85)
    property color highlightForegroundColor: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.85)
    property color highlightBarColor: Kirigami.Theme.highlightColor

    property color pressedColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.3)
    property color hoverSelectColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.2)
    property color checkedBorderColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.7)
    property color pressedBorderColor: Qt.rgba(highlightBarColor.r, highlightBarColor.g, highlightBarColor.b, 0.9)

    property url source
    property string name

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

        implicitHeight: (control.display === T.AbstractButton.TextBesideIcon) ? 0 : (Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2)

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

    contentItem: GridLayout {
        id: gridLayout
        columnSpacing: 0
        rowSpacing: (label.visible && label.lineCount > 1) ? 0 : control.spacing

        // if this is a row or a column
        columns: control.display !== T.AbstractButton.TextBesideIcon ? 1 : 2

        property real verticalMargins: (control.display === T.AbstractButton.TextBesideIcon) ? Kirigami.Units.largeSpacing : 0

        Kirigami.Avatar {
            id: icon
            source: control.source
            name: control.name
            Layout.topMargin: gridLayout.verticalMargins
            Layout.bottomMargin: gridLayout.verticalMargins
            Layout.leftMargin: (control.display === T.AbstractButton.TextBesideIcon) ? Kirigami.Units.gridUnit : 0
            Layout.rightMargin: (control.display === T.AbstractButton.TextBesideIcon) ? Kirigami.Units.gridUnit : 0

            Layout.alignment: {
                if (control.display === T.AbstractButton.TextBesideIcon) {
                    // row layout
                    return Qt.AlignVCenter | Qt.AlignRight;
                } else {
                    // column layout
                    return Qt.AlignHCenter | ((!label.visible || label.lineCount > 1) ? Qt.AlignVCenter : Qt.AlignBottom);
                }
            }
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        }
        QQC2.Label {
            id: label
            Kirigami.MnemonicData.enabled: control.enabled && control.visible
            Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.MenuItem
            Kirigami.MnemonicData.label: control.text

            text: Kirigami.MnemonicData.richTextLabel
            horizontalAlignment: (control.display === T.AbstractButton.TextBesideIcon) ? Text.AlignLeft : Text.AlignHCenter

            visible: control.display !== T.AbstractButton.IconOnly
            wrapMode: Text.Wrap
            elide: Text.ElideMiddle
            color: control.checked ? control.highlightForegroundColor : control.foregroundColor

            font.bold: control.checked
            font.family: Kirigami.Theme.smallFont.family
            font.pointSize: {
                if (control.display === T.AbstractButton.TextBesideIcon) {
                    // row layout
                    return Kirigami.Theme.defaultFont.pointSize;
                } else {
                    // column layout
                    return icon.visible ? Kirigami.Theme.smallFont.pointSize : Kirigami.Theme.defaultFont.pointSize * 1.20; // 1.20 is equivalent to level 2 heading
                }
            }

            Behavior on color { ColorAnimation {} }
            Behavior on opacity { NumberAnimation {} }

            Layout.topMargin: gridLayout.verticalMargins
            Layout.bottomMargin: gridLayout.verticalMargins

            Layout.alignment: {
                if (control.display === T.AbstractButton.TextBesideIcon) {
                    // row layout
                    return Qt.AlignVCenter | Qt.AlignLeft;
                } else {
                    // column layout
                    return icon.visible ? Qt.AlignHCenter | Qt.AlignTop : Qt.AlignCenter;
                }
            }

            // Work around bold text changing implicit size
            Layout.preferredWidth: boldMetrics.implicitWidth
            Layout.preferredHeight: boldMetrics.implicitHeight * label.lineCount
            Layout.fillWidth: true

            QQC2.Label {
                id: boldMetrics
                visible: false
                text: parent.text
                font.bold: true
                font.family: Kirigami.Theme.smallFont.family
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                horizontalAlignment: Text.AlignHCenter
                wrapMode: QQC2.Label.Wrap
                elide: Text.ElideMiddle
            }
        }
    }
}