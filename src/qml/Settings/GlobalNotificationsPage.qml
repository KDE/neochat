// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "Notifications")
    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        id: notificationLayout

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: MobileForm.FormCheckDelegate {
                text: i18n("Enable notifications for this account")
                checked: Config.showNotifications
                enabled: !Config.isShowNotificationsImmutable && Controller.activeConnection
                onToggled: {
                    Config.showNotifications = checked
                    Config.save()
                    NotificationsManager.globalNotificationsEnabled = checked
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
                NotificationRuleItem {
                    text: i18n("Messages in one-to-one chats")

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.oneToOneNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.oneToOneNotificationAction)
                    enabled: NotificationsManager.oneToOneNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.oneToOneNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.oneToOneNotificationAction != notificationAction) {
                            NotificationsManager.oneToOneNotificationAction = notificationAction
                        }
                    }
                }
                NotificationRuleItem {
                    text: i18n("Encrypted messages in one-to-one chats")

                    visible: Controller.encryptionSupported

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.encryptedOneToOneNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.encryptedOneToOneNotificationAction)
                    enabled: NotificationsManager.encryptedOneToOneNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.encryptedOneToOneNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.encryptedOneToOneNotificationAction != notificationAction) {
                            NotificationsManager.encryptedOneToOneNotificationAction = notificationAction
                        }
                    }
                }
                NotificationRuleItem {
                    text: i18n("Messages in group chats")

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.groupChatNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.groupChatNotificationAction)
                    enabled: NotificationsManager.groupChatNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.groupChatNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.groupChatNotificationAction != notificationAction) {
                            NotificationsManager.groupChatNotificationAction = notificationAction
                        }
                    }
                }
                NotificationRuleItem {
                    text: i18n("Messages in encrypted group chats")

                    visible: Controller.encryptionSupported

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.encryptedGroupChatNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.encryptedGroupChatNotificationAction)
                    enabled: NotificationsManager.encryptedGroupChatNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.encryptedGroupChatNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.encryptedGroupChatNotificationAction != notificationAction) {
                            NotificationsManager.encryptedGroupChatNotificationAction = notificationAction
                        }
                    }
                }
                NotificationRuleItem {
                    text: i18n("Room upgrade messages")

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.tombstoneNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.tombstoneNotificationAction)
                    highlightable: true
                    highlightOn: notificationLayout.isNotificationRuleHighlight(NotificationsManager.tombstoneNotificationAction)
                    enabled: NotificationsManager.tombstoneNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.tombstoneNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.tombstoneNotificationAction != notificationAction) {
                            NotificationsManager.tombstoneNotificationAction = notificationAction
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
                    title: i18n("@Mentions")
                }
                NotificationRuleItem {
                    text: i18n("Messages containing my display name")

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.displayNameNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.displayNameNotificationAction)
                    highlightable: true
                    highlightOn: notificationLayout.isNotificationRuleHighlight(NotificationsManager.displayNameNotificationAction)
                    enabled: NotificationsManager.displayNameNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.displayNameNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.displayNameNotificationAction != notificationAction) {
                            NotificationsManager.displayNameNotificationAction = notificationAction
                        }
                    }
                }
                NotificationRuleItem {
                    text: i18n("Whole room (@room) notifications")

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.roomNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.roomNotificationAction)
                    highlightable: true
                    highlightOn: notificationLayout.isNotificationRuleHighlight(NotificationsManager.roomNotificationAction)
                    enabled: NotificationsManager.roomNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.roomNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.roomNotificationAction != notificationAction) {
                            NotificationsManager.roomNotificationAction = notificationAction
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
                    title: i18n("Keywords")
                }
                NotificationRuleItem {
                    id: keywordNotificationAction
                    text: i18n("Messages containing my keywords")

                    notificationsOn: true
                    notificationsOnModifiable: false
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.keywordNotificationAction)
                    highlightable: true
                    highlightOn: notificationLayout.isNotificationRuleHighlight(NotificationsManager.keywordNotificationAction)
                    enabled: NotificationsManager.keywordNotificationAction !== PushNotificationAction.Unknown &&
                             NotificationsManager.keywordNotificationAction !== PushNotificationAction.Off

                    notificationAction: NotificationsManager.keywordNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.keywordNotificationAction != notificationAction) {
                            NotificationsManager.keywordNotificationAction = notificationAction
                        }
                    }
                }
                MobileForm.FormDelegateSeparator {}
                Repeater {
                    model: KeywordNotificationRuleModel {
                        id: keywordNotificationRuleModel
                    }

                    delegate: NotificationRuleItem {
                        text: name
                        notificationAction: keywordNotificationAction.notificationAction
                        notificationsOn: keywordNotificationAction.notificationsOn
                        notificationsOnModifiable: false
                        noisyOn: keywordNotificationAction.noisyOn
                        noisyModifiable: false
                        highlightOn: keywordNotificationAction.highlightOn
                        deletable: true

                        onDeleteItemChanged: {
                            if (deleteItem && deletable) {
                                keywordNotificationRuleModel.removeKeywordAtIndex(index)
                            }
                        }
                    }
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
                                keywordNotificationRuleModel.addKeyword(keywordAddField.text, PushNotificationAction.On)
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
                                keywordNotificationRuleModel.addKeyword(keywordAddField.text, PushNotificationAction.On)
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
                NotificationRuleItem {
                    text: i18n("Invites to a room")

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.inviteNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.inviteNotificationAction)
                    highlightable: true
                    highlightOn: notificationLayout.isNotificationRuleHighlight(NotificationsManager.inviteNotificationAction)
                    enabled: NotificationsManager.inviteNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.inviteNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.inviteNotificationAction != notificationAction) {
                            NotificationsManager.inviteNotificationAction = notificationAction
                        }
                    }
                }
                NotificationRuleItem {
                    text: i18n("Call invitation")

                    // TODO enable this option when calls are supported
                    visible: false

                    notificationsOn: notificationLayout.isNotificationRuleOn(NotificationsManager.callInviteNotificationAction)
                    noisyOn: notificationLayout.isNotificationRuleNoisy(NotificationsManager.callInviteNotificationAction)
                    highlightable: true
                    highlightOn: notificationLayout.isNotificationRuleHighlight(NotificationsManager.callInviteNotificationAction)
                    enabled: NotificationsManager.callInviteNotificationAction !== PushNotificationAction.Unknown

                    notificationAction: NotificationsManager.callInviteNotificationAction
                    onNotificationActionChanged: {
                        if (notificationAction && NotificationsManager.callInviteNotificationAction != notificationAction) {
                            NotificationsManager.callInviteNotificationAction = notificationAction
                        }
                    }
                }
            }
        }

        function isNotificationRuleOn(action) {
            return action == PushNotificationAction.On ||
                action == PushNotificationAction.Noisy ||
                action == PushNotificationAction.Highlight ||
                action == PushNotificationAction.NoisyHighlight
        }

        function isNotificationRuleNoisy(action) {
            return action == PushNotificationAction.Noisy ||
                action == PushNotificationAction.NoisyHighlight
        }

        function isNotificationRuleHighlight(action) {
            return action == PushNotificationAction.Highlight ||
                action == PushNotificationAction.NoisyHighlight
        }
    }
}
