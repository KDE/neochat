// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title:window", "Notifications")
    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        id: notificationLayout

        MobileForm.FormCard {
            Layout.fillWidth: true
            contentItem: MobileForm.FormCheckDelegate {
                text: i18n("Enable notifications for this account")
                description: i18n("Whether push notifications are generated by your Matrix server")
                checked: Controller.pushRuleModel.globalNotificationsEnabled
                enabled: Controller.pushRuleModel.globalNotificationsSet
                onToggled: {
                    Controller.pushRuleModel.globalNotificationsEnabled = checked
                }
            }
        }

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Room Notifications")
                }
                Repeater {
                    model: KSortFilterProxyModel {
                        sourceModel: Controller.pushRuleModel

                        filterRowCallback: function(source_row, source_parent) {
                            let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole)
                            return sectionRole == PushNotificationSection.Room;
                        }
                    }

                    delegate: ruleDelegate
                }
            }
        }

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("@Mentions")
                }
                Repeater {
                    model: KSortFilterProxyModel {
                        sourceModel: Controller.pushRuleModel

                        filterRowCallback: function(source_row, source_parent) {
                            let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole)
                            return sectionRole == PushNotificationSection.Mentions;
                        }
                    }

                    delegate: ruleDelegate
                }
            }
        }

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Keywords")
                }
                Repeater {
                    model: KSortFilterProxyModel {
                        sourceModel: Controller.pushRuleModel

                        filterRowCallback: function(source_row, source_parent) {
                            let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole)
                            return sectionRole == PushNotificationSection.Keywords;
                        }
                    }

                    delegate: ruleDelegate
                }
                MobileForm.AbstractFormDelegate {
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
                                Controller.pushRuleModel.addKeyword(keywordAddField.text)
                                keywordAddField.text = ""
                            }
                        }
                        QQC2.Button {
                            id: addButton

                            text: i18n("Add keyword")
                            Accessible.name: text
                            icon.name: "list-add"
                            display: QQC2.AbstractButton.IconOnly
                            enabled: NotificationsManager.keywordNotificationAction !== PushNotificationAction.Unknown

                            onClicked: {
                                Controller.pushRuleModel.addKeyword(keywordAddField.text)
                                keywordAddField.text = ""
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

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Invites")
                }
                Repeater {
                    model: KSortFilterProxyModel {
                        sourceModel: Controller.pushRuleModel

                        filterRowCallback: function(source_row, source_parent) {
                            let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole)
                            return sectionRole == PushNotificationSection.Invites;
                        }
                    }

                    delegate: ruleDelegate
                }
            }
        }
    }

    Component {
        id: ruleDelegate
        NotificationRuleItem {
            onDeleteRule: {
                Controller.pushRuleModel.removeKeyword(id)
            }
            onActionChanged: (action) => Controller.pushRuleModel.setPushRuleAction(id, action)
        }
    }
}
