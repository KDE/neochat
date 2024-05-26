// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Import Keys")

    header: KirigamiComponents.Banner {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.FormButtonDelegate {
            id: fileButton
            text: i18nc("@action:button", "Choose backup file")
            description: ""
            icon.name: "cloud-upload"
            onClicked: {
                let dialog = Qt.createComponent("org.kde.neochat", "OpenFileDialog").createObject(root);
                dialog.chosen.connect(path => {
                    fileButton.description = path.substring(7);
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
                if (error === KeyImport.Success) {
                    root.closeDialog();
                } else if (error === KeyImport.InvalidPassphrase) {
                    banner.text = i18nc("@info", "Invalid passphrase");
                    banner.visible = true;
                } else if (error === KeyImport.InvalidData) {
                    banner.text = i18nc("@info", "Invalid key backup data");
                    banner.visible = true;
                } else {
                    banner.text = i18nc("@info", "Unknown error");
                    banner.visible = true;
                }
            }
        }
    }


}
