// SPDX-FileCopyrightText: 2020 - 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    title: i18n("Devices")

    required property NeoChatConnection connection

    property DevicesModel devicesModel: DevicesModel {
        id: devicesModel
        connection: root.connection
    }

    DevicesCard {
        title: i18n("This Device")
        type: DevicesModel.This
        showVerifyButton: false
    }
    DevicesCard {
        title: i18n("Verified Devices")
        type: DevicesModel.Verified
        showVerifyButton: true
    }
    DevicesCard {
        title: i18n("Unverified Devices")
        type: DevicesModel.Unverified
        showVerifyButton: true
    }
    DevicesCard {
        title: i18n("Devices without Encryption Support")
        type: DevicesModel.Unencrypted
        showVerifyButton: false
    }

    FormCard.AbstractFormDelegate {
        Layout.fillWidth: true
        visible: Controller.activeConnection && devicesModel.count === 0 // We can assume 0 means loading since there is at least one device
        contentItem: Kirigami.LoadingPlaceholder { }
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Please login to view the signed-in devices for your account.")
        type: Kirigami.MessageType.Information
        visible: !Controller.activeConnection
    }

    property Kirigami.OverlaySheet passwordSheet: Kirigami.OverlaySheet {
        id: passwordSheet

        property string deviceId

        title: i18n("Remove device")
        Kirigami.FormLayout {
            QQC2.TextField {
                id: passwordField
                Kirigami.FormData.label: i18n("Password:")
                echoMode: TextInput.Password
            }
            QQC2.Button {
                text: i18n("Confirm")
                onClicked: {
                    devicesModel.logout(passwordSheet.deviceId, passwordField.text)
                    passwordField.text = ""
                    passwordSheet.close()
                }
            }
        }
    }
}
