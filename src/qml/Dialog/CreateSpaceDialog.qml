// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18n("Create a Space")

    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    FormCard.FormHeader {
        title: i18nc("@title", "Create a Space")
    }
    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameDelegate
            label: i18n("Space name")
        }
        FormCard.FormTextFieldDelegate {
            id: topicDelegate
            label: i18n("Space topic (optional)")
        }
        FormCard.FormButtonDelegate {
            text: i18n("Create space")
            onClicked: {
                root.connection.createSpace(nameDelegate.text, topicDelegate.text)
                root.close()
                root.destroy()
            }
            enabled: nameDelegate.text.length > 0
        }
    }
}
