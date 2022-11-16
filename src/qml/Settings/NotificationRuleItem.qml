// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

MobileForm.AbstractFormDelegate {
    id: notificationRuleItem

    property var notificationAction: PushNotificationAction.Unkown
    property bool notificationsOn: false
    property bool notificationsOnModifiable: true
    property bool noisyOn: false
    property bool noisyModifiable: true
    property bool highlightOn: false
    property bool highlightable: false
    property bool deleteItem: false
    property bool deletable: false

    Layout.fillWidth: true

    onClicked: {
        notificationAction = nextNotificationRuleAction(notificationAction)
    }

    contentItem : RowLayout {
        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            Layout.minimumWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
            Layout.minimumHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing

            text: notificationsOn ? "" : "●"
            color: Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            background: Rectangle {
                visible: notificationsOn
                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                color: highlightOn ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                opacity: highlightOn ? 1 : 0.3
                radius: height / 2
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            text: notificationRuleItem.text
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            maximumLineCount: 2
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight

            QQC2.Button {
                id: onButton

                text: onButton.checked ? i18n("Disable notifications") : i18n("Enable notifications")
                Accessible.name: text
                icon.name: checked ? "notifications" : "notifications-disabled"
                display: QQC2.AbstractButton.IconOnly

                visible: notificationRuleItem.notificationsOnModifiable
                checkable: true
                checked: notificationRuleItem.notificationsOn
                enabled: notificationRuleItem.enabled
                down: checked
                onToggled: {
                    notificationRuleItem.notificationAction = notificationRuleItem.notifcationRuleAction()
                }

                QQC2.ToolTip {
                    text: onButton.text
                    delay: Kirigami.Units.toolTipDelay
                }
            }
            QQC2.Button {
                id: noisyButton

                text: noisyButton.checked ? i18n("Mute notifications") : i18n("Unmute notifications")
                Accessible.name: text
                icon.name: checked ? "audio-volume-high" : "audio-volume-muted"
                display: QQC2.AbstractButton.IconOnly

                visible: notificationRuleItem.noisyModifiable
                checkable: true
                checked: notificationRuleItem.noisyOn
                enabled: (onButton.checked || !notificationRuleItem.notificationsOnModifiable) && notificationRuleItem.enabled
                down: checked
                onToggled: {
                    notificationRuleItem.notificationAction = notificationRuleItem.notifcationRuleAction()
                }

                QQC2.ToolTip {
                    text: noisyButton.text
                    delay: Kirigami.Units.toolTipDelay
                }
            }
            QQC2.Button {
                id: highlightButton

                text: highlightButton.checked ? i18nc("As in clicking this button will switch off highlights for messages that match this rule", "Disable message highlights") : i18nc("As in clicking this button will switch on highlights for messages that match this rule", "Enable message highlights")
                Accessible.name: text
                icon.name: "draw-highlight"
                display: QQC2.AbstractButton.IconOnly

                visible: notificationRuleItem.highlightable
                checkable: true
                checked: notificationRuleItem.highlightOn
                enabled: (onButton.checked || !notificationRuleItem.notificationsOnModifiable) && notificationRuleItem.enabled
                down: checked
                onToggled: {
                    notificationRuleItem.notificationAction = notificationRuleItem.notifcationRuleAction()
                }

                QQC2.ToolTip {
                    text: highlightButton.text
                    delay: Kirigami.Units.toolTipDelay
                }
            }
            QQC2.Button {
                id: deleteButton

                Accessible.name: i18n("Delete keyword")
                icon.name: "edit-delete-remove"

                visible: notificationRuleItem.deletable

                onClicked: {
                    notificationRuleItem.deleteItem = !notificationRuleItem.deleteItem
                }
            }
        }
    }

    function notifcationRuleAction() {
        if (onButton.checked) {
            if (noisyButton.checked && highlightButton.checked) {
                return PushNotificationAction.NoisyHighlight
            } else if (noisyButton.checked) {
                return PushNotificationAction.Noisy
            } else if (highlightButton.checked) {
                return PushNotificationAction.Highlight
            } else {
                return PushNotificationAction.On
            }
        } else {
            return PushNotificationAction.Off
        }
    }

    function nextNotificationRuleAction(action) {
        let finished = false

        if (action == PushNotificationAction.NoisyHighlight) {
            action = PushNotificationAction.Off
        } else {
            action += 1
        }

        while (!finished) {
            if (action == PushNotificationAction.Off && !notificationRuleItem.notificationsOnModifiable) {
                action = PushNotificationAction.On
            } else if (action == PushNotificationAction.Noisy && !notificationRuleItem.noisyModifiable) {
                action = PushNotificationAction.Highlight
            } else if (action == PushNotificationAction.Highlight && !notificationRuleItem.highlightable) {
                action = PushNotificationAction.Off
            } else {
                finished = true
            }
        }

        return action
    }
}
