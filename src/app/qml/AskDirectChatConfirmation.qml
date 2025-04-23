// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.PromptDialog {
    id: root

    required property var user

    title: i18nc("@title:dialog", "Start a chat")
    subtitle: i18n("Do you want to start a chat with %1?", root.user.displayName)
    dialogType: Kirigami.PromptDialog.Warning

    onRejected: {
        root.close();
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.Dialog.Cancel

        QQC2.Button {
            text: i18nc("@action:button", "Start Chat")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            icon.name: "im-user"
            onClicked: {
                root.user.requestDirectChat();
                root.close();
            }
        }
    }
}
