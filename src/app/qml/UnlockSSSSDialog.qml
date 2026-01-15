// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property bool processing: false

    title: i18nc("@title:window", "Manage Secret Backup")

    topPadding: Kirigami.Units.gridUnit
    leftPadding: 0
    rightPadding: 0

    header: Kirigami.InlineMessage {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
        position: Kirigami.InlineMessage.Position.Header
    }

    Connections {
        target: Controller.activeConnection
        function onKeyBackupError(): void {
            securityKeyField.clear()
            root.processing = false
            banner.text = i18nc("@info:status", "The security key or backup passphrase was not correct.")
            banner.visible = true
        }

        function onKeyBackupUnlocked(): void {
            root.processing = false
            banner.text = i18nc("@info:status", "Encryption keys restored.")
            banner.type = Kirigami.MessageType.Positive
            banner.visible = true
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Unlock using Security Key or Backup Passphrase")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            description: i18nc("@info", "If you have a security key or backup passphrase for this account, enter it below or upload it as a file.")
        }
        FormCard.FormTextFieldDelegate {
            id: securityKeyField
            label: i18nc("@label:textbox", "Security Key or Backup Passphrase:")
            echoMode: TextInput.Password
        }
        FormCard.FormButtonDelegate {
            id: uploadSecurityKeyButton
            text: i18nc("@action:button", "Upload from File")
            icon.name: "cloud-upload"
            enabled: !root.processing
            onClicked: {
                root.processing = true
                openFileDialog.open()
            }
        }
        FormCard.FormButtonDelegate {
            id: unlockSecurityKeyButton
            text: i18nc("@action:button", "Unlock")
            icon.name: "unlock"
            enabled: securityKeyField.text.length > 0 && !root.processing
            onClicked: {
                root.processing = true
                Controller.activeConnection.unlockSSSS(securityKeyField.text)
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
            enabled: !root.processing
            onClicked: {
                root.processing = true
                Controller.activeConnection.unlockSSSS("")
            }
        }
    }

    property OpenFileDialog openFileDialog: OpenFileDialog {
        id: openFileDialog
        onChosen: path => securityKeyField.text = Controller.loadFileContent(path)
    }
}
