// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room

    title: i18nc('@title:window', 'Notifications')

    FormCard.FormHeader {
        title: i18n("Room notifications setting")
    }

    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18n("Follow global setting")
            checked: room.pushNotificationState === PushNotificationState.Default
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.Default
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'notify for all messages'","All")
            checked: room.pushNotificationState === PushNotificationState.All
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.All
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'","@Mentions and Keywords")
            checked: room.pushNotificationState === PushNotificationState.MentionKeyword
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.MentionKeyword
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'do not notify for any messages'","Off")
            checked: room.pushNotificationState === PushNotificationState.Mute
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.Mute
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Keywords")
    }

    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: Controller.pushRuleModel

                filterRowCallback: function(source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole)
                    let roomIdRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.RoomIdRole)
                    return sectionRole == PushNotificationSection.RoomKeywords && roomIdRole == root.room.id;
                }
            }

            delegate: ruleDelegate
        }

        FormCard.AbstractFormDelegate {
            Layout.fillWidth: true

            contentItem : RowLayout {
                Kirigami.ActionTextField {
                    id: keywordAddField

                    Layout.fillWidth: true

                    placeholderText: i18n("Keyword…")
                    enabled: NotificationsManager.keywordNotificationAction !== PushNotificationAction.Unknown

                    rightActions: Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: keywordAddField.text.length > 0
                        onTriggered: {
                            keywordAddField.text = ""
                        }
                    }

                    onAccepted: {
                        Controller.pushRuleModel.addKeyword(keywordAddField.text, root.room.id)
                        keywordAddField.text = ""
                    }
                }
                QQC2.Button {
                    id: addButton

                    text: i18n("Add keyword")
                    icon.name: "list-add"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: NotificationsManager.keywordNotificationAction !== PushNotificationAction.Unknown

                    onClicked: {
                        Controller.pushRuleModel.addKeyword(keywordAddField.text, root.room.id)
                        keywordAddField.text = ""
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }
    }

    data: Component {
        id: ruleDelegate
        NotificationRuleItem {
            onDeleteRule: {
                Controller.pushRuleModel.removeKeyword(id)
            }
            onActionChanged: (action) => Controller.pushRuleModel.setPushRuleAction(id, action)
        }
    }
}
