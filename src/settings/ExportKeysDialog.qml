// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Export Keys")

    required property NeoChatConnection connection

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.FormTextFieldDelegate {
            id: passphraseField
            label: i18nc("@label:textbox", "Passphrase:")
            echoMode: TextInput.Password
        }
        FormCard.FormButtonDelegate {
            enabled: passphraseField.text.length > 0
            text: i18nc("@action:button", "Export keys")
            onClicked: {
                let dialog = saveDialog.createObject(root);
                dialog.accepted.connect(() => {
                    root.connection.exportMegolmSessions(passphraseField.text, dialog.selectedFile);
                });
                dialog.open();
            }
        }
    }

    Component {
        id: saveDialog
        FileDialog {
            fileMode: FileDialog.SaveFile
            currentFolder: Config.lastSaveDirectory.length > 0 ? Config.lastSaveDirectory : StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        }
    }
}
