// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

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

    readonly property PushRuleModel pushRuleModel: PushRuleModel {
        connection: root.connection
    }
    
    actions: [
        Kirigami.Action {
            displayComponent: QQC2.Switch {
                text: i18nc("@option:check Whether notifications are enabled for this account", "Enabled for this account")
                checkable: true
                checked: root.pushRuleModel.globalNotificationsEnabled
                enabled: root.pushRuleModel.globalNotificationsSet
                onToggled: root.pushRuleModel.globalNotificationsEnabled = checked
            }
        }
    ]

    background: Item {
        Kirigami.PlaceholderMessage {
            icon.name: "notifications"
            text: i18nc("@info:placeholder", "Notifications Disabled")
            visible: !root.pushRuleModel.globalNotificationsEnabled

            anchors.centerIn: parent
        }
    }

    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled

        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormSwitchDelegate {
            id: enableDeviceNotificationsDelegate

            text: i18nc("@option:check", "Enable notifications for this device")
            checked: root.connection.enableDeviceNotifications
            onToggled: root.connection.enableDeviceNotifications = checked
        }
        FormCard.FormDelegateSeparator {
            visible: root.connection.enableDeviceNotifications
            above: enableDeviceNotificationsDelegate
            below: configureSystemNotificationsDelegate
        }
        FormCard.FormButtonDelegate {
            id: configureSystemNotificationsDelegate

            icon.name: "configure-symbolic"
            text: i18nc("@action:button", "Configure System Notifications…")
            visible: root.connection.enableDeviceNotifications && root.connection.inKDESession()

            // TODO: replace with proper KNotifications API in the future and when we can depend on it!
            onClicked: Qt.openUrlExternally("systemsettings:/kcm_notifications/--notifyrc neochat")
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group Background/push notifications", "Background Notifications")
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications && root.connection.pushNotificationsAvailable
    }
    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications && root.connection.pushNotificationsAvailable

        FormCard.AbstractFormDelegate {
            id: pushNotificationsStatusDelegate

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing
                Kirigami.Icon {
                    source: {
                        if (root.connection.pushNotificationsAvailable && !root.connection.enablePushNotifications) {
                            "data-warning"
                        } else {
                            "data-information"
                        }
                    }
                }
                QQC2.Label {
                    text: {
                        if (root.connection.pushNotificationsAvailable) {
                            if (root.connection.enablePushNotifications) {
                                return i18nc("@info:label", "Notifications can appear even when NeoChat isn't running.");
                            } else {
                                return i18nc("@info:label", "The configured push distributor does not have a Matrix gateway.");
                            }
                        }
                        return ""; // this section would be hidden anyway!
                    }
                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                }
            }
        }
        FormCard.FormDelegateSeparator {
            above: pushNotificationsStatusDelegate
            below: backgroundNotificationsDelegate
            visible: root.connection.inKDESession()
        }
        FormCard.FormButtonDelegate {
            id: backgroundNotificationsDelegate

            icon.name: "configure-symbolic"
            text: i18nc("@action:button", "Configure Background Notifications…")
            visible: root.connection.inKDESession()

            onClicked: Qt.openUrlExternally("systemsettings:/kcm_push_notifications")
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Room Notifications")
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications
    }
    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications

        FormCard.AbstractFormDelegate {
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing
                Kirigami.Icon {
                    source: "data-information"
                }
                QQC2.Label {
                    text: i18nc("@info", "These are the default notification settings for all rooms. You can customize notifications per-room in the room list or room settings.")
                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                }
            }
        }
    }
    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications

        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Room;
                }
            }

            delegate: root.ruleDelegate
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group Mention notifications", "Mentions")
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications
    }
    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications

        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Mentions;
                }
            }

            delegate: root.ruleDelegate
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Keywords")
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications
    }
    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications

        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel

                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Keywords;
                }
            }

            delegate: root.ruleDelegate
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
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications
    }
    FormCard.FormCard {
        visible: root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications

        Repeater {
            model: KSortFilterProxyModel {
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Invites;
                }
            }

            delegate: root.ruleDelegate
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Unknown")
        visible: unknownModel.rowCount() > 0 && root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications
    }
    FormCard.FormCard {
        visible: unknownModel.rowCount() > 0 && root.pushRuleModel.globalNotificationsEnabled && root.connection.enableDeviceNotifications

        Repeater {
            model: KSortFilterProxyModel {
                id: unknownModel
                sourceModel: root.pushRuleModel
                filterRowCallback: function (source_row, source_parent) {
                    let sectionRole = sourceModel.data(sourceModel.index(source_row, 0, source_parent), PushRuleModel.SectionRole);
                    return sectionRole == PushRuleSection.Unknown;
                }
            }

            delegate: root.ruleDelegate
        }
    }

    readonly property Component ruleDelegate: Component {
        NotificationRuleItem {
            onDeleteRule: {
                root.pushRuleModel.removeKeyword(id);
            }
            onNotificatonActionChanged: action => root.pushRuleModel.setPushRuleAction(id, action)
        }
    }
}
