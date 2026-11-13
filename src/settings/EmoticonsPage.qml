// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.ki18n

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: KI18n.i18nc("@title:window", "Stickers & Emojis")

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Emojis")
    }
    EmoticonFormCard {
        emoticonType: EmoticonFormCard.Emojis
        connection: root.connection
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Stickers")
    }
    EmoticonFormCard {
        emoticonType: EmoticonFormCard.Stickers
        connection: root.connection
    }
}
