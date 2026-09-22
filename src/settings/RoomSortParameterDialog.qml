// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Dialog {
    id: root
    title: i18nc("@title:dialog", "Custom Room Sort Order")

    width: Math.min(parent.width, Kirigami.Units.gridUnit * 24)
    height: Math.min(parent.height, Kirigami.Units.gridUnit * 24)

    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    Component.onCompleted: {
        (header as QQC2.Control).background.children[0].visible = true
        (footer as QQC2.Control).background.children[0].visible = true
    }

    onAccepted: {
        NeoChatConfig.customSortOrder = roomSortParameterModel.currentParameterList;
        NeoChatConfig.sortOrder = RoomSortOrder.Custom;
        NeoChatConfig.save()
    }

    contentItem: QQC2.ScrollView {
        clip: true

        ListView {
            implicitHeight: contentHeight
            currentIndex: -1

            model: RoomSortParameterModel {
                id: roomSortParameterModel
            }

            delegate: QQC2.ItemDelegate {
                id: parameterDelegate
                required property string name
                required property string description
                required property int index

                width: ListView.view.width

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    ColumnLayout {
                        spacing: Kirigami.Units.smallSpacing
                        Layout.fillWidth: true

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: parameterDelegate.index == 0 ? i18nc("As in first sort chat rooms by the parameter", "first:") : i18nc("As in then sort chat rooms by the parameter", "then:")
                        }
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            text: parameterDelegate.name
                            level: 4
                        }
                        QQC2.Label {
                            Layout.fillWidth: true
                            text: parameterDelegate.description
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            wrapMode: Text.Wrap
                        }
                    }
                    QQC2.ToolButton {
                        text: i18nc("@button", "Up")
                        icon.name: "arrow-up"
                        display: QQC2.AbstractButton.IconOnly
                        onClicked: roomSortParameterModel.moveRowUp(parameterDelegate.index)

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    QQC2.ToolButton {
                        text: i18nc("@button", "Down")
                        icon.name: "arrow-down"
                        display: QQC2.AbstractButton.IconOnly
                        onClicked: roomSortParameterModel.moveRowDown(parameterDelegate.index)

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    QQC2.ToolButton {
                        text: i18nc("@button", "Remove")
                        icon.name: "list-remove"
                        display: QQC2.AbstractButton.IconOnly
                        onClicked: roomSortParameterModel.removeRow(parameterDelegate.index)

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                }
            }

            footer: QQC2.ItemDelegate {
                text: i18nc("@action:button", "Add parameter")
                icon.name: "list-add"

                onClicked: (addParameterDialogComponent.createObject(root) as Kirigami.Dialog).open()
            }
        }
    }

    Component {
        id: addParameterDialogComponent

        Kirigami.Dialog {
            id: addParameterDialog
            title: i18nc("@title:dialog", "Select Parameter to Add")

            width: Math.min(parent.width, Kirigami.Units.gridUnit * 24)
            height: Math.min(parent.height, Kirigami.Units.gridUnit * 24)

            standardButtons: Kirigami.Dialog.Cancel

            Component.onCompleted: {
                (header as QQC2.Control).background.children[0].visible = true
                (footer as QQC2.Control).background.children[0].visible = true
            }

            contentItem: QQC2.ScrollView {
                clip: true

                ListView {
                    implicitHeight: contentHeight
                    currentIndex: -1

                    model: roomSortParameterModel.allParameterModel()

                    delegate: QQC2.ItemDelegate {
                        id: addParameterDelegate
                        required property string name
                        required property string description
                        required property int index

                        width: parent?.width ?? 0

                        text: addParameterDelegate.name

                        contentItem: Kirigami.TitleSubtitle {
                            title: addParameterDelegate.text
                            subtitle: addParameterDelegate.description
                            font: addParameterDelegate.font
                            selected: addParameterDelegate.highlighted || addParameterDelegate.down
                        }

                        onClicked: {
                            roomSortParameterModel.addParameter(addParameterDelegate.index)
                            addParameterDialog.close()
                        }
                    }
                }
            }
        }
    }
}
