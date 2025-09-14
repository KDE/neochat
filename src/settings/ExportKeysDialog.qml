// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Export Keys")

    required property NeoChatConnection connection

    header: Kirigami.InlineMessage {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
        position: Kirigami.InlineMessage.Position.Header
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.FormTextFieldDelegate {
            id: passphraseField
            label: i18nc("@label:textbox", "Passphrase:")
            echoMode: TextInput.Password
        }
        FormCard.FormTextDelegate {
            text: i18n("A passphrase to secure your key backup. It should not be your account password.")
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            enabled: passphraseField.text.length > 0
            text: i18nc("@action:button", "Export keys")
            onClicked: {
                let dialog = saveDialog.createObject(root);
                dialog.accepted.connect(() => {
                    banner.visible = false;
                    let error = root.connection.exportMegolmSessions(passphraseField.text, dialog.selectedFile);
                    passphraseField.text = "";
                    if (error === KeyImport.Success) {
                        banner.text = i18nc("@info", "Keys exported successfully");
                        banner.type = Kirigami.MessageType.Positive;
                        banner.visible = true;
                    } else {
                        banner.text = i18nc("@info", "Unknown error");
                        banner.type = Kirigami.MessageType.Error;
                        banner.visible = true;
                    }
                });
                dialog.open();
            }
        }
    }

    Component {
        id: saveDialog
        FileDialog {
            fileMode: FileDialog.SaveFile
            currentFolder: NeoChatConfig.lastSaveDirectory.length > 0 ? NeoChatConfig.lastSaveDirectory : StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        }
    }
}
