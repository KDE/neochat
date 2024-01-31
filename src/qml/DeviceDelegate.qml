// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.AbstractFormDelegate {
    id: root

    required property string id
    required property string timestamp
    required property string displayName

    property bool editDeviceName: false
    property bool showVerifyButton

    onClicked: root.editDeviceName = true

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            source: "network-connect"
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium
        }
        ColumnLayout {
            id: deviceLabel
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: !root.editDeviceName

            QQC2.Label {
                Layout.fillWidth: true
                text: root.displayName
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: i18nc("@label", "%1, Last activity: %2", root.id, root.timestamp)
                color: Kirigami.Theme.disabledTextColor
                font: Kirigami.Theme.smallFont
                elide: Text.ElideRight
                visible: text.length > 0
            }
        }
        Kirigami.ActionTextField {
            id: nameField
            Accessible.description: i18n("New device name")
            Layout.fillWidth: true
            Layout.preferredHeight: deviceLabel.implicitHeight
            visible: root.editDeviceName

            text: root.displayName

            rightActions: [
                Kirigami.Action {
                    text: i18n("Cancel editing display name")
                    icon.name: "edit-delete-remove"
                    onTriggered: {
                        root.editDeviceName = false;
                    }
                },
                Kirigami.Action {
                    text: i18n("Confirm new display name")
                    icon.name: "checkmark"
                    visible: nameField.text !== root.displayName
                    onTriggered: {
                        devicesModel.setName(root.id, nameField.text);
                    }
                }
            ]

            onAccepted: devicesModel.setName(root.id, nameField.text)
        }
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            action: Kirigami.Action {
                id: editDeviceAction
                text: i18n("Edit device name")
                icon.name: "document-edit"
                onTriggered: root.editDeviceName = true
            }
            QQC2.ToolTip {
                text: editDeviceAction.text
                delay: Kirigami.Units.toolTipDelay
            }
        }
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            visible: root.showVerifyButton
            action: Kirigami.Action {
                id: verifyDeviceAction
                text: i18n("Verify device")
                icon.name: "security-low-symbolic"
                onTriggered: {
                    devicesModel.connection.startKeyVerificationSession(devicesModel.connection.localUserId, root.id);
                }
            }
            QQC2.ToolTip {
                text: verifyDeviceAction.text
                delay: Kirigami.Units.toolTipDelay
            }
        }
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            action: Kirigami.Action {
                id: logoutDeviceAction
                text: i18n("Logout device")
                icon.name: "edit-delete-remove"
                onTriggered: {
                    passwordSheet.deviceId = root.id;
                    passwordSheet.open();
                }
            }
            QQC2.ToolTip {
                text: logoutDeviceAction.text
                delay: Kirigami.Units.toolTipDelay
            }
        }
    }
}
