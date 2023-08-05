// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

MobileForm.AbstractFormDelegate {
    id: deviceDelegate

    required property string id
    required property string timestamp
    required property string displayName

    property bool editDeviceName: false
    property bool showVerifyButton

    Layout.fillWidth: true

    onClicked: deviceDelegate.editDeviceName = true

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
            visible: !deviceDelegate.editDeviceName

            QQC2.Label {
                Layout.fillWidth: true
                text: deviceDelegate.displayName
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: i18nc("@label", "%1, Last activity: %2", deviceDelegate.id, deviceDelegate.timestamp)
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
            visible: deviceDelegate.editDeviceName

            text: deviceDelegate.displayName

            rightActions: [
                Kirigami.Action {
                    text: i18n("Cancel editing display name")
                    icon.name: "edit-delete-remove"
                    onTriggered: {
                        deviceDelegate.editDeviceName = false
                    }
                },
                Kirigami.Action {
                    text: i18n("Confirm new display name")
                    icon.name: "checkmark"
                    visible: nameField.text !== deviceDelegate.displayName
                    onTriggered: {
                        devicesModel.setName(deviceDelegate.id, nameField.text)
                    }
                }
            ]

            onAccepted: devicesModel.setName(deviceDelegate.id, nameField.text)
        }
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            action: Kirigami.Action {
                id: editDeviceAction
                text: i18n("Edit device name")
                icon.name: "document-edit"
                onTriggered: deviceDelegate.editDeviceName = true
            }
            QQC2.ToolTip {
                text: editDeviceAction.text
                delay: Kirigami.Units.toolTipDelay
            }
        }
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            visible: Controller.encryptionSupported && deviceDelegate.showVerifyButton
            action: Kirigami.Action {
                id: verifyDeviceAction
                text: i18n("Verify device")
                icon.name: "security-low-symbolic"
                onTriggered: {
                    devicesModel.connection.startKeyVerificationSession(devicesModel.connection.localUserId, deviceDelegate.id)
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
                    passwordSheet.deviceId = deviceDelegate.id
                    passwordSheet.open()
                }
            }
            QQC2.ToolTip {
                text: logoutDeviceAction.text
                delay: Kirigami.Units.toolTipDelay
            }
        }
    }
}
