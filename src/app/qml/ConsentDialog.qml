// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.PromptDialog {
    id: root

    required property string url

    title: i18nc("@title:dialog", "User Consent")
    subtitle: i18nc("@info", "Your homeserver requires you to agree to its terms and conditions before being able to use it. Please click the button below to read them.")
    dialogType: Kirigami.PromptDialog.Warning

    onRejected: {
        root.close();
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.Dialog.Cancel

        QQC2.Button {
            text: i18nc("@action:button", "Open")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            icon.name: "internet-services"
            onClicked: {
                UrlHelper.openUrl(root.url);
                root.close();
            }
        }
    }
}
