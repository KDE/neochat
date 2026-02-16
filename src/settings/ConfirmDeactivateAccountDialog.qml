// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami as Kirigami
import org.kde.neochat

Kirigami.PromptDialog {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:dialog", "Confirm Account Deactivation")
    subtitle: i18n("Your account will be permanently disabled.\nThis cannot be undone.\nYour Matrix ID will not be available for new accounts.\nYour messages will stay available.")

    dialogType: Kirigami.PromptDialog.Warning
    standardButtons: QQC2.Dialog.Cancel

    onAccepted: root.connection.deactivateAccount(passwordField.text, eraseDelegate.checked)

    mainItem: ColumnLayout {
        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18nc("@label:textbox", "Password")
            echoMode: TextInput.Password
            horizontalPadding: 0
            bottomPadding: 0
        }

        FormCard.FormCheckDelegate {
            id: eraseDelegate
            text: i18nc("@label:checkbox", "Erase Data")
            description: i18nc("@info", "Request your server to delete as much user data as possible.")
            visible: connection.canEraseData
        }
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18n("Deactivate account")
            icon.name: "emblem-warning"
            enabled: passwordField.text.length > 0

            onClicked: root.accept()

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
    }
}
