// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:window", "Notifications")

    property PushRuleModel pushRuleModel: PushRuleModel {
        connection: root.connection
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4
        FormCard.FormCheckDelegate {
            text: i18n("Enable notifications for this account")
            description: {
                if (connection.pushNotificationsAvailable) {
                    if (connection.enablePushNotifications) {
                        return i18n("Notifications can appear even when NeoChat isn't running.");
                    } else {
                        return i18n("Push notifications are available but could not be enabled.");
                    }
                } else {
                    return i18n("Notifications will only appear when NeoChat is running.");
                }
            }
            checked: root.pushRuleModel.globalNotificationsEnabled
            enabled: root.pushRuleModel.globalNotificationsSet
            onToggled: {
                root.pushRuleModel.globalNotificationsEnabled = checked;
            }
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.AbstractFormDelegate {
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing
                Kirigami.Icon {
                    source: "data-information"
                    width: Kirigami.Units.iconSizes.sizeForLabels
                    height: Kirigami.Units.iconSizes.sizeForLabels
                }
                QQC2.Label {
                    text: i18nc("@info", "These are the default notification settings for all rooms. You can customize notifications per-room in the room list or room settings.")
                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Room Notifications")
    }
    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Room;
                }
            }

            delegate: ruleDelegate
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "@Mentions")
    }
    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Mentions;
                }
            }

            delegate: ruleDelegate
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Keywords")
    }
    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel

                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Keywords;
                }
            }

            delegate: ruleDelegate
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
                        root.pushRuleModel.addKeyword(keywordAddField.text);
                        keywordAddField.text = "";
                    }
                }
                QQC2.Button {
                    id: addButton

                    text: i18n("Add keyword")
                    Accessible.name: text
                    icon.name: "list-add"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: NotificationsManager.keywordNotificationAction !== PushRuleAction.Unknown

                    onClicked: {
                        root.pushRuleModel.addKeyword(keywordAddField.text);
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

    FormCard.FormHeader {
        title: i18nc("@title:group", "Invites")
    }
    FormCard.FormCard {
        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Invites;
                }
            }

            delegate: ruleDelegate
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Unknown")
        visible: unknownModel.rowCount() > 0
    }
    FormCard.FormCard {
        visible: unknownModel.rowCount() > 0

        Repeater {
            model: KSortFilterProxyModel {
                id: unknownModel
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Unknown;
                }
            }

            delegate: ruleDelegate
        }
    }

    property Component ruleDelegate: Component {
        id: ruleDelegate
        NotificationRuleItem {
            onDeleteRule: {
                root.pushRuleModel.removeKeyword(id);
            }
            onNotificatonActionChanged: action => root.pushRuleModel.setPushRuleAction(id, action)
        }
    }
}
