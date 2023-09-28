// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

Item {
    id: root
    required property TreeView treeView
    required property bool isTreeNode
    required property bool expanded
    required property int hasChildren
    required property int depth
    required property string roomId
    required property string displayName
    required property url avatarUrl
    required property bool isSpace
    required property int memberCount
    required property string topic
    required property bool isJoined
    required property bool canAddChildren

    signal createRoom()
    signal enterRoom()

    Delegates.RoundedItemDelegate {
        anchors.centerIn: root
        width: sizeHelper.currentWidth

        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing

            RowLayout {
                spacing: 0
                Item {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium * (root.depth + (root.isSpace ? 0 : 1))
                }
                Kirigami.Icon {
                    visible: root.isSpace
                    implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    implicitHeight: Kirigami.Units.iconSizes.smallMedium
                    source: root.hasChildren ? (root.expanded ? "go-up" : "go-down") : "go-next"
                }
            }
            Components.Avatar {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                implicitWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                implicitHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                source: root.avatarUrl
                name: root.displayName
            }
            ColumnLayout {
                spacing: 0

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    spacing: Kirigami.Units.largeSpacing
                    QQC2.Label {
                        id: label
                        text: root.displayName
                        elide: Text.ElideRight
                        textFormat: Text.PlainText
                    }
                    QQC2.Label {
                        visible: root.isJoined
                        text: i18n("Joined")
                        color: Kirigami.Theme.linkColor
                    }
                }
                QQC2.Label {
                    id: subtitle
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                    text: root.memberCount + (root.topic !== "" ? i18nc("number of room members", " members - ") + root.topic : i18nc("number of room members", " members"))
                    elide: Text.ElideRight
                    font: Kirigami.Theme.smallFont
                    textFormat: Text.PlainText
                    maximumLineCount: 1
                }
            }
            QQC2.ToolButton {
                visible: root.isSpace && root.canAddChildren
                text: i18nc("@button", "Add new child")
                icon.name: "list-add"
                onClicked: root.createRoom()

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        TapHandler {
            onTapped: {
                if (root.isSpace) {
                    root.treeView.toggleExpanded(row)
                } else {
                    if (root.isJoined) {
                        root.enterRoom()
                    } else {
                        Controller.joinRoom(root.roomId)
                    }
                }
            }
        }
    }

    DelegateSizeHelper {
        id: sizeHelper
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: 85
        maxWidth: Kirigami.Units.gridUnit * 60

        parentWidth: root.treeView ? root.treeView.width : 0
    }
}
