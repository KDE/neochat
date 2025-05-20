// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.PromptDialog {
    id: root

    required property NeoChatRoom room

    title: i18nc("@title:dialog", "Confirm Leaving Room")
    subtitle: root.room ? i18nc("Do you really want to leave <room name>?", "Do you really want to leave %1?", root.room.displayNameForHtml) : ""
    dialogType: Kirigami.PromptDialog.Warning

    onRejected: {
        root.close();
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.Dialog.Cancel

        QQC2.Button {
            text: i18nc("@action:button", "Leave Room")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            icon.name: "arrow-left-symbolic"
            onClicked: root.room.forget();
        }
    }
}
