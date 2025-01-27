// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:dialog", "Upload Logs")

    FormCard.FormTextDelegate {
        text: i18nc("@info", "Uploading NeoChat's logs can help with finding bugs in the app. After uploading the logs, a token will be shown which identifies the file you have uploaded. Please add this token when creating a bug report for your problem. Uploaded logs are only accessible to KDE developers and will never contain secret data.");
        textItem.wrapMode: Text.Wrap
    }

    FormCard.FormTextAreaDelegate {
        id: description
        label: i18nc("@label", "Description")
        placeholderText: i18nc("@info:placeholder", "Things are not working")
    }

    QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18nc("@action:button", "Upload")
            //TODO icon
        }
        standardButtons: QQC2.DialogButtonBox.Cancel
    }
}
