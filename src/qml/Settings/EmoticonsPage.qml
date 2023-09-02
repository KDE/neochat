// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Stickers & Emojis")

    FormCard.FormHeader {
        title: i18n("Emojis")
    }
    EmoticonFormCard {
        emoticonType: EmoticonFormCard.Emojis
    }

    FormCard.FormHeader {
        title: i18n("Stickers")
    }
    EmoticonFormCard {
        emoticonType: EmoticonFormCard.Stickers
    }


    property Component emoticonEditorPage: Component {
        id: emoticonEditorPage
        EmoticonEditorPage {}
    }
}
