// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Security")

    FormCard.FormHeader {
        title: i18nc("@title", "Keys")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: connection.deviceKey
            description: i18n("Device key")
        }
        FormCard.FormTextDelegate {
            text: connection.encryptionKey
            description: i18n("Encryption key")
        }
        FormCard.FormTextDelegate {
            text: connection.deviceId
            description: i18n("Device id")
        }
    }
}
