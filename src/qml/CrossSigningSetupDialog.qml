// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.PromptDialog {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:dialog", "Setup Encryption Keys")
    dialogType: Kirigami.PromptDialog.Information

    onRejected: {
        root.close();
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.Dialog.Cancel

        QQC2.Button {
            text: i18nc("@action:button", "Setup Keys")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: {
                root.connection.setupCrossSigningKeys("password123");
            }
        }
    }
}
