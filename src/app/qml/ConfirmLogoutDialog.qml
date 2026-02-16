// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.PromptDialog {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:dialog", "Sign out")
    subtitle: i18n("Are you sure you want to sign out?")
    dialogType: Kirigami.PromptDialog.Warning
    standardButtons: QQC2.Dialog.Cancel

    onAccepted: root.connection.logout(true)

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18nc("@action:button", "Sign out")
            onClicked: root.accept()

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
    }
}
