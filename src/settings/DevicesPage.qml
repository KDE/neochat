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

    property QQC2.Dialog passwordSheet: QQC2.Dialog {
        id: passwordSheet

        property string deviceId

        preferredWidth: Kirigami.Units.gridUnit * 24

        title: i18n("Remove device")

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: passwordField
                label: i18n("Password:")
                echoMode: TextInput.Password
            }
        }
        footer: QQC2.DialogButtonBox {
            standardButtons: QQC2.Dialog.Cancel

            QQC2.Button {
                text: i18nc("As in 'Remove this device'", "Remove")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                icon.name: "delete"
                onClicked: {
                    devicesModel.logout(passwordSheet.deviceId, passwordField.text);
                    passwordField.text = "";
                    passwordSheet.close();
                }
            }
        }
    }
}
