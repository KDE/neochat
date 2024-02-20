// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: root

    required property TreeView treeView
    required property bool isTreeNode
    required property bool expanded
    required property bool hasChildren
    required property int depth
    required property string displayName
    required property int row
    required property bool current
    onCurrentChanged: if (current) {
        collapseButton.forceActiveFocus(Qt.TabFocusReason);
    }
    required property bool selected

    property bool collapsed: false

    implicitWidth: treeView.width

    hoverEnabled: false
    activeFocusOnTab: false
    background: null

    onClicked: root.treeView.toggleExpanded(row)

    Keys.onEnterPressed: root.treeView.toggleExpanded(row)
    Keys.onReturnPressed: root.treeView.toggleExpanded(row)
    Keys.onSpacePressed: root.treeView.toggleExpanded(row)

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.alignment: Qt.AlignVCenter
            visible: !root.collapsed

            opacity: 0.7
            level: 5
            type: Kirigami.Heading.Primary
            text: root.collapsed ? "" : model.displayName
            elide: Text.ElideRight

            // we override the Primary type's font weight (DemiBold) for Bold for contrast with small text
            font.weight: Font.Bold
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: !root.collapsed
            Layout.alignment: Qt.AlignVCenter
        }
        QQC2.ToolButton {
            id: collapseButton
            Layout.alignment: Qt.AlignHCenter

            icon {
                name: root.expanded ? "go-up" : "go-down"
                width: Kirigami.Units.iconSizes.small
                height: Kirigami.Units.iconSizes.small
            }
            text: root.expanded ? i18nc("Collapse <section name>", "Collapse %1", root.displayName) : i18nc("Expand <section name", "Expand %1", root.displayName)
            display: QQC2.Button.IconOnly

            activeFocusOnTab: false

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            onClicked: root.treeView.toggleExpanded(row)
        }
    }
}
