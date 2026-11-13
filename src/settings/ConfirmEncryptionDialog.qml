// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.ki18n

Kirigami.PromptDialog {
    id: root

    title: KI18n.i18nc("@title:dialog", "Activate Encryption")
    subtitle: KI18n.i18n("It will not be possible to deactivate the encryption after it is enabled.")
    dialogType: Kirigami.PromptDialog.Warning
    standardButtons: QQC2.Dialog.Cancel

    property NeoChatRoom room

    onAccepted: root.room.activateEncryption()

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: KI18n.i18n("Activate Encryption")

            onClicked: root.accept()

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
    }
}
