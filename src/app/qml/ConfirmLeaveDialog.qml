// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.ki18n

Kirigami.PromptDialog {
    id: root

    required property NeoChatRoom room

    title: root.room.isSpace ? KI18n.i18nc("@title:dialog", "Confirm Leaving Space") : KI18n.i18nc("@title:dialog", "Confirm Leaving Room")
    subtitle: root.room ? KI18n.i18nc("Do you really want to leave <room name>?", "Do you really want to leave %1?", root.room.displayNameForHtml) : ""
    dialogType: Kirigami.PromptDialog.Warning
    standardButtons: QQC2.Dialog.Cancel

    onAccepted: root.room.forget()

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: KI18n.i18nc("@action:button Leave this room/space", "Leave")
            icon.name: "arrow-left-symbolic"

            onClicked: root.accept()

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
    }
}
