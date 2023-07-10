// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    title: emoticonType === EmoticonFormCard.Emojis ? i18n("Emojis") : i18n("Stickers")
    topPadding: 0
    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        spacing: 0
        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("Emojis")
        }
        EmoticonFormCard {
            emoticonType: EmoticonFormCard.Emojis
        }
        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("Stickers")
        }
        EmoticonFormCard {
            emoticonType: EmoticonFormCard.Stickers
        }
    }

    Component {
        id: emoticonEditorPage
        EmoticonEditorPage {}
    }
}
