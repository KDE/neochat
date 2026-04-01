// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property NeoChatRoom room

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title: create new poll in the room", "Create Poll")
    showCloseButton: false
    standardButtons: QQC2.Dialog.Cancel

    onAccepted: root.room.postPoll(pollTypeCombo.currentValue, questionTextField.text, optionModel.values())

    contentItem: ColumnLayout {
        spacing: 0

        FormCard.FormComboBoxDelegate {
            id: pollTypeCombo

            text: i18nc("@label The type of poll to create", "Show Results:")
            currentIndex: 0
            textRole: "text"
            valueRole: "value"
            model: [
                { value: PollKind.Disclosed, text: i18nc("@item:inlistbox Show results after a user votes", "After Voting") },
                { value: PollKind.Undisclosed, text: i18nc("@item:inlistbox Show results after the poll ends", "After Poll Ends") }
            ]
        }
        FormCard.AbstractFormDelegate {
            id: anonymousWarningDelegate

            contentItem: Kirigami.InlineMessage {
                showCloseButton: false
                visible: true
                type: Kirigami.MessageType.Information
                position: Kirigami.InlineMessage.Position.Inline
                text: i18nc("@info", "Votes are not anonymous and will be visible to everyone in the room.")
            }
        }
        FormCard.FormDelegateSeparator {
            above: anonymousWarningDelegate
            below: questionTextField
        }
        FormCard.FormTextFieldDelegate {
            id: questionTextField
            label: i18nc("@label", "Question:")
            placeholderText: i18nc("@info:placeholder Example poll question", "What's your favorite food?")
        }
        FormCard.FormDelegateSeparator {
            above: questionTextField
        }
        Repeater {
            id: optionRepeater

            model: ListModel {
                id: optionModel

                readonly property bool allValuesSet: {
                    for (let i = 0; i < optionModel.rowCount(); i++) {
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
                    for(let i = 0; i < optionModel.rowCount(); i++) {
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
                            placeholderText: i18nc("@placeholder", "Enter option")
                        }
                        QQC2.ToolButton {
                            display: QQC2.AbstractButton.IconOnly
                            text: i18nc("@action:button", "Remove option")
                            icon.name: "edit-delete-remove"
                            enabled: optionRepeater.count > 2 // Don't allow removing the last two options

                            onClicked: optionModel.remove(optionDelegate.index)

                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            QQC2.ToolTip.visible: hovered
                        }
                    }
                }

                background: null
            }
        }
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            enabled: optionModel.allValuesSet && questionTextField.text.length > 0
            text: i18nc("@action:button", "Send")
            icon.name: "document-send"

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }

        QQC2.Button {
            icon.name: "list-add"
            text: i18nc("@action:button Add new poll option", "Add Option")

            onClicked: optionModel.append({optionText: ""})

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.ActionRole
        }
    }
}
