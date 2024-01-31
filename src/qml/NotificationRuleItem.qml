// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.AbstractFormDelegate {
    id: root

    required property string id
    required property string name
    required property int ruleAction
    required property bool highlightable
    required property bool deletable

    readonly property bool notificationsOn: isNotificationRuleOn(ruleAction)
    readonly property bool notificationsOnModifiable: !deletable
    readonly property bool highlightOn: isNotificationRuleHighlight(ruleAction)

    signal actionChanged(int action)
    signal deleteRule

    enabled: ruleAction !== PushRuleAction.Unknown

    text: name

    onClicked: {
        notificationAction = nextNotificationRuleAction(notificationAction);
    }

    contentItem: RowLayout {
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
                color: highlightOn && highlightable ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                opacity: highlightOn && highlightable ? 1 : 0.3
                radius: height / 2
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            text: root.text
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

                visible: root.notificationsOnModifiable
                checkable: true
                checked: root.notificationsOn
                enabled: root.enabled
                down: checked
                onToggled: {
                    root.actionChanged(root.notifcationRuleAction());
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

                checkable: true
                checked: isNotificationRuleNoisy(root.ruleAction)
                enabled: (onButton.checked || !root.notificationsOnModifiable) && root.enabled
                down: checked
                onToggled: {
                    root.actionChanged(root.notifcationRuleAction());
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

                visible: root.highlightable
                checkable: true
                checked: root.highlightOn
                enabled: (onButton.checked || !root.notificationsOnModifiable) && root.enabled
                down: checked
                onToggled: {
                    root.actionChanged(root.notifcationRuleAction());
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

                visible: root.deletable

                onClicked: {
                    root.deleteRule();
                }
            }
        }
    }

    function notifcationRuleAction() {
        if (onButton.checked) {
            if (noisyButton.checked && highlightButton.checked && root.highlightable) {
                return PushRuleAction.NoisyHighlight;
            } else if (noisyButton.checked) {
                return PushRuleAction.Noisy;
            } else if (highlightButton.checked && root.highlightable) {
                return PushRuleAction.Highlight;
            } else {
                return PushRuleAction.On;
            }
        } else {
            return PushRuleAction.Off;
        }
    }

    function nextNotificationRuleAction(action) {
        let finished = false;
        if (action == PushRuleAction.NoisyHighlight) {
            action = PushRuleAction.Off;
        } else {
            action += 1;
        }
        while (!finished) {
            if (action == PushRuleAction.Off && !root.notificationsOnModifiable) {
                action = PushRuleAction.On;
            } else if (action == PushRuleAction.Noisy) {
                action = PushRuleAction.Highlight;
            } else if (action == PushRuleAction.Highlight && !root.highlightable) {
                action = PushRuleAction.Off;
            } else {
                finished = true;
            }
        }
        actionChanged(action);
    }

    function isNotificationRuleOn(action) {
        return action == PushRuleAction.On || action == PushRuleAction.Noisy || action == PushRuleAction.Highlight || action == PushRuleAction.NoisyHighlight;
    }

    function isNotificationRuleNoisy(action) {
        return action == PushRuleAction.Noisy || action == PushRuleAction.NoisyHighlight;
    }

    function isNotificationRuleHighlight(action) {
        return action == PushRuleAction.Highlight || action == PushRuleAction.NoisyHighlight;
    }
}
