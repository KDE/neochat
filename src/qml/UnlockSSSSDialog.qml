// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import com.github.quotient_im.libquotient

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Load your encrypted messages")

    topPadding: Kirigami.Units.gridUnit
    leftPadding: 0
    rightPadding: 0

    header: KirigamiComponents.Banner {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
    }

    property SSSSHandler ssssHandler: SSSSHandler {
        id: ssssHandler

        property bool processing: false

        connection: Controller.activeConnection
        onKeyBackupUnlocked: {
            ssssHandler.processing = false
            root.closeDialog()
        }
        onError: error => {
            if (error !== SSSSHandler.WrongKeyError) {
                banner.text = error
                banner.visible = true
                return;
            }
            passwordField.clear()
            ssssHandler.processing = false
            banner.text = i18nc("@info:status", "The security phrase was not correct.")
            banner.visible = true
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Unlock using Passphrase")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            description: i18nc("@info", "If you have a backup passphrase for this account, enter it below.")
        }
        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18nc("@label:textbox", "Backup Password:")
            echoMode: TextInput.Password
        }
        FormCard.FormButtonDelegate {
            id: unlockButton
            text: i18nc("@action:button", "Unlock")
            icon.name: "unlock"
            enabled: passwordField.text.length > 0 && !ssssHandler.processing
            onClicked: {
                ssssHandler.processing = true
                banner.visible = false
                ssssHandler.unlockSSSSWithPassphrase(passwordField.text)
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Unlock using Security Key")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            description: i18nc("@info", "If you have a security key for this account, enter it below or upload it as a file.")
        }
        FormCard.FormTextFieldDelegate {
            id: securityKeyField
            label: i18nc("@label:textbox", "Security Key:")
            echoMode: TextInput.Password
        }
        FormCard.FormButtonDelegate {
            id: uploadSecurityKeyButton
            text: i18nc("@action:button", "Upload from File")
            icon.name: "cloud-upload"
            enabled: !ssssHandler.processing
            onClicked: {
                ssssHandler.processing = true
                openFileDialog.open()
            }
        }
        FormCard.FormButtonDelegate {
            id: unlockSecurityKeyButton
            text: i18nc("@action:button", "Unlock")
            icon.name: "unlock"
            enabled: securityKeyField.text.length > 0 && !ssssHandler.processing
            onClicked: {
                ssssHandler.processing = true
                ssssHandler.unlockSSSSFromSecurityKey(securityKeyField.text)
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Unlock from Cross-Signing")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            description: i18nc("@info", "If you have previously verified this device, you can try loading the backup key from other devices by clicking the button below.")
        }
        FormCard.FormButtonDelegate {
            id: unlockCrossSigningButton
            icon.name: "emblem-shared-symbolic"
            text: i18nc("@action:button", "Request from other Devices")
            enabled: !ssssHandler.processing
            onClicked: {
                ssssHandler.processing = true
                ssssHandler.unlockSSSSFromCrossSigning()
            }
        }
    }

    property OpenFileDialog openFileDialog: OpenFileDialog {
        id: openFileDialog
        onChosen: securityKeyField.text = Controller.loadFileContent(path)
    }
}
