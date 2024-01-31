// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room

    property PushRuleModel pushRuleModel: PushRuleModel {
        connection: root.room.connection
    }

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
                room.pushNotificationState = PushNotificationState.Default;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'notify for all messages'", "All")
            checked: room.pushNotificationState === PushNotificationState.All
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.All;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'", "@Mentions and Keywords")
            checked: room.pushNotificationState === PushNotificationState.MentionKeyword
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.MentionKeyword;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'do not notify for any messages'", "Off")
            checked: room.pushNotificationState === PushNotificationState.Mute
            enabled: room.pushNotificationState !== PushNotificationState.Unknown
            onToggled: {
                room.pushNotificationState = PushNotificationState.Mute;
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Keywords")
    }
    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel

                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    let roomIdRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.RoomIdRole);
                    return sectionRole == PushRuleSection.RoomKeywords && roomIdRole == root.room.id;
                }
            }

            delegate: ruleDelegate

            Component {
                id: ruleDelegate
                NotificationRuleItem {
                    onDeleteRule: {
                        root.pushRuleModel.removeKeyword(id);
                    }
                    onActionChanged: action => root.pushRuleModel.setPushRuleAction(id, action)
                }
            }
        }
        FormCard.AbstractFormDelegate {
            Layout.fillWidth: true

            contentItem: RowLayout {
                Kirigami.ActionTextField {
                    id: keywordAddField

                    Layout.fillWidth: true

                    placeholderText: i18n("Keyword…")
                    enabled: NotificationsManager.keywordNotificationAction !== PushRuleAction.Unknown

                    rightActions: Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: keywordAddField.text.length > 0
                        onTriggered: {
                            keywordAddField.text = "";
                        }
                    }

                    onAccepted: {
                        root.pushRuleModel.addKeyword(keywordAddField.text, root.room.id);
                        keywordAddField.text = "";
                    }
                }
                QQC2.Button {
                    id: addButton

                    text: i18n("Add keyword")
                    Accessible.name: text
                    icon.name: "list-add"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: NotificationsManager.keywordNotificationAction !== PushRuleAction.Unknown && keywordAddField.text.length > 0

                    onClicked: {
                        root.pushRuleModel.addKeyword(keywordAddField.text, root.room.id);
                        keywordAddField.text = "";
                    }

                    QQC2.ToolTip {
                        text: addButton.text
                        delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }
    }
}
