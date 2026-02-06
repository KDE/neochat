// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import io.github.quotient_im.libquotient
import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    signal success

    title: i18nc("@title:window", "Import Keys")

    header: Kirigami.InlineMessage {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
        position: Kirigami.InlineMessage.Position.Header
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.FormButtonDelegate {
            id: fileButton
            text: i18nc("@action:button", "Choose backup file")
            description: ""
            icon.name: "cloud-upload"
            onClicked: {
                let dialog = Qt.createComponent("QtQuick.Dialogs", "FileDialog").createObject(root, {
                    fileMode: FileDialog.OpenFile
                });
                dialog.accepted.connect(() => {
                    fileButton.description = dialog.selectedFile.toString().substring(7);
                });
                dialog.open();
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: passphraseField
            label: i18nc("@label:textbox", "Passphrase:")
            echoMode: TextInput.Password
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Import keys")
            enabled: fileButton.description.length > 0 && passphraseField.text.length > 0
            onClicked: {
                banner.visible = false;
                let error = KeyImport.importKeys(Controller.loadFileContent(fileButton.description), passphraseField.text, root.connection);
                passphraseField.text = "";
                if (error === KeyImport.Success) {
                    root.success();
                    root.closeDialog();
                } else if (error === KeyImport.InvalidPassphrase) {
                    banner.text = i18nc("@info", "Invalid passphrase");
                    banner.type = Kirigami.MessageType.Error;
                    banner.visible = true;
                } else if (error === KeyImport.InvalidData) {
                    banner.text = i18nc("@info", "Invalid key backup data");
                    banner.type = Kirigami.MessageType.Error;
                    banner.visible = true;
                } else {
                    banner.text = i18nc("@info", "Unknown error");
                    banner.type = Kirigami.MessageType.Error;
                    banner.visible = true;
                }
            }
        }
    }


}
