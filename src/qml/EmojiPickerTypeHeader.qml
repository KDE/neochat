// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.NavigationTabBar {
    id: root

    enum EmoticonType {
        Emoji,
        Sticker
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    property var selectedType: EmojiPickerTypeHeader.EmoticonType.Emoji

    background: null
    actions: [
        Kirigami.Action {
            id: emojis
            icon.name: "smiley"
            text: i18n("Emojis")
            checked: root.selectedType === EmojiPickerTypeHeader.EmoticonType.Emoji

            onTriggered: root.selectedType = EmojiPickerTypeHeader.EmoticonType.Emoji
        },
        Kirigami.Action {
            id: stickers
            icon.name: "stickers"
            text: i18n("Stickers")
            checked: root.selectedType === EmojiPickerTypeHeader.EmoticonType.Sticker
            onTriggered: root.selectedType = EmojiPickerTypeHeader.EmoticonType.Sticker
        }
    ]
}
