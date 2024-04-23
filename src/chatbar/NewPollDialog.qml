// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import Quotient

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property NeoChatRoom room

    standardButtons: Kirigami.Dialog.Cancel

    customFooterActions: [
        Kirigami.Action {
            enabled: optionModel.allValuesSet && questionTextField.text.length > 0
            text: i18nc("@action:button", "Send")
            icon.name: "document-send"
            onTriggered: {
                root.room.postPoll(pollTypeCombo.currentValue, questionTextField.text, optionModel.values())
                root.close()
            }
        }
    ]

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title: create new poll in the room", "Create Poll")

    contentItem: ColumnLayout {
        spacing: 0

        FormCard.FormComboBoxDelegate {
            id: pollTypeCombo

            text: i18n("Poll type:")
            currentIndex: 0
            textRole: "text"
            valueRole: "value"
            model: [
                { value: PollKind.Disclosed, text: i18n("Open poll") },
                { value: PollKind.Undisclosed, text: i18n("Closed poll") }
            ]
        }
        FormCard.FormTextDelegate {
            verticalPadding: 0
            text: pollTypeCombo.currentValue == 0 ? i18n("Voters can see the result as soon as they have voted") : i18n("Results are revealed only after the poll has closed")
        }
        FormCard.FormTextFieldDelegate {
            id: questionTextField
            label: i18n("Question:")
        }
        Repeater {
            id: optionRepeater

            model: ListModel {
                id: optionModel

                readonly property bool allValuesSet: {
                    for( var i = 0; i < optionModel.rowCount(); i++ ) {
                        if (optionModel.get(i).optionText.length <= 0) {
                            return false;
                        }
                    }
                    return true;
                }

                ListElement {
                    optionText: ""
                }
                ListElement {
                    optionText: ""
                }

                function values() {
                    let textValues = []
                    for( var i = 0; i < optionModel.rowCount(); i++ ) {
                        textValues.push(optionModel.get(i).optionText);
                    }
                    return textValues;
                }
            }
            delegate: FormCard.AbstractFormDelegate {
                id: optionDelegate

                required property int index
                required property string optionText

                contentItem: ColumnLayout {
                    QQC2.Label {
                        id: optionLabel

                        Layout.fillWidth: true
                        text: i18nc("As in first answer option to the poll", "Option %1:", optionDelegate.index + 1)
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        Accessible.ignored: true
                    }
                    RowLayout {
                        Layout.fillWidth: true

                        QQC2.TextField {
                            id: textField
                            Layout.fillWidth: true
                            Accessible.name: optionLabel.text
                            onTextChanged: {
                                optionModel.set(optionDelegate.index, {optionText: text})
                                optionModel.allValuesSetChanged()
                            }
                            placeholderText: i18n("Enter option")
                        }
                        QQC2.ToolButton {
                            display: QQC2.AbstractButton.IconOnly
                            action: Kirigami.Action {
                                id: removeOptionAction
                                text: i18nc("@action:button", "Remove option")
                                icon.name: "edit-delete-remove"
                                onTriggered: optionModel.remove(optionDelegate.index)
                            }
                            QQC2.ToolTip {
                                text: removeOptionAction.text
                                delay: Kirigami.Units.toolTipDelay
                            }
                        }
                    }
                }

                background: null
            }
        }
        Delegates.RoundedItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: Kirigami.Units.largeSpacing * 2
            leftInset: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            rightInset: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            highlighted: true

            icon.name: "list-add"
            text: i18nc("@action:button", "Add option")

            onClicked: optionModel.append({optionText: ""})
        }
    }
}
