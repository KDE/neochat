// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Devices")

    background: Kirigami.PlaceholderMessage {
        text: i18n("Loading…")
        visible: !thisDeviceCard.visible
    }

    required property NeoChatConnection connection

    property DevicesModel devicesModel: DevicesModel {
        id: devicesModel
        connection: root.connection
    }

    DevicesCard {
        id: thisDeviceCard
        title: i18nc("@info:group", "This Device")
        type: DevicesModel.This
        showVerifyButton: false

        FormCard.FormButtonDelegate {
            icon.name: "security-low"
            text: i18nc("@action:button", "Verify This Device")
            description: i18nc("@info:description", "This device is marked as insecure until it's verified by another device. It's recommended to verify as soon as possible.")
            visible: !root.connection.isVerifiedSession()
            onClicked: {
                root.connection.startSelfVerification();
                const dialog = Qt.createComponent("org.kde.kirigami", "PromptDialog").createObject(QQC2.Overlay.overlay, {
                    title: i18nc("@title", "Verification Request Sent"),
                    subtitle: i18nc("@info:label", "To proceed, accept the verification request on another device."),
                    standardButtons: Kirigami.Dialog.Ok
                })
                dialog.open();
                root.connection.onNewKeyVerificationSession.connect(() => {
                    dialog.close();
                });
            }
        }
    }
    DevicesCard {
        title: i18nc("@info:group", "Verified Devices")
        type: DevicesModel.Verified
        showVerifyButton: true
    }
    DevicesCard {
        title: i18nc("@info:group", "Unverified Devices")
        type: DevicesModel.Unverified
        showVerifyButton: true
    }
    DevicesCard {
        title: i18nc("@info:group", "Devices without Encryption Support")
        type: DevicesModel.Unencrypted
        showVerifyButton: false
    }

    FormCard.AbstractFormDelegate {
        Layout.fillWidth: true
        visible: root.connection && devicesModel.count === 0 // We can assume 0 means loading since there is at least one device
        contentItem: Kirigami.LoadingPlaceholder {}
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Please login to view the signed-in devices for your account.")
        type: Kirigami.MessageType.Information
        visible: !root.connection
    }

    property Kirigami.Dialog passwordSheet: Kirigami.Dialog {
        id: passwordSheet

        property string deviceId

        preferredWidth: Kirigami.Units.gridUnit * 24

        title: i18n("Remove device")

        standardButtons: QQC2.Dialog.Cancel
        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: passwordField
                label: i18n("Password:")
                echoMode: TextInput.Password
            }
        }
        customFooterActions: [
            Kirigami.Action {
                text: i18nc("As in 'Remove this device'", "Remove")
                icon.name: "delete"
                onTriggered: {
                    devicesModel.logout(passwordSheet.deviceId, passwordField.text);
                    passwordField.text = "";
                    passwordSheet.close();
                }
            }
        ]
    }
}
