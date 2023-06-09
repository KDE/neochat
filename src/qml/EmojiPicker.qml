// SPDX-FileCopyrightText: 2022-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.neochat

ColumnLayout {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    property NeoChatRoom currentRoom

    signal chosen(string emoji)

    spacing: 0

    onActiveFocusChanged: if (activeFocus) {
        searchField.forceActiveFocus();
    }

    EmojiPickerTypeHeader {
        id: emoticonPickerTypeHeader

        Layout.fillWidth: true
        onSelectedTypeChanged: emoticonPickerCategoryHeader.currentIndex = 0
    }

    EmojiPickerPackHeader {
        id: emoticonPickerCategoryHeader

        Layout.fillWidth: true

        model: ImagePacksProxyModel {
            currentRoom: root.currentRoom
            stickers: emoticonPickerTypeHeader.selectedType === EmojiPickerTypeHeader.EmoticonType.Sticker
            emojis: emoticonPickerTypeHeader.selectedType === EmojiPickerTypeHeader.EmoticonType.Emoji
        }
    }

    Kirigami.Separator {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }

    Kirigami.SearchField {
        id: searchField
        Layout.margins: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        focusSequence: ""
    }

    EmojiGrid {
        id: emojiGrid
        model: ImageContentFilterModel {
            id: imageContentFilterModel
            sourceModel: searchField.text.length > 0 ? emojiSearchModel
                : emoticonPickerCategoryHeader.currentCategory === "history" ? recentImageContentProxyModel
                : imageContentModel
            stickers: emoticonPickerTypeHeader.selectedType === EmojiPickerTypeHeader.EmoticonType.Sticker
            emojis: emoticonPickerTypeHeader.selectedType === EmojiPickerTypeHeader.EmoticonType.Emoji
        }
        Layout.fillWidth: true
        Layout.fillHeight: true
        onChosen: unicode => root.chosen(unicode)
        header: emoticonPickerCategoryHeader
        Keys.forwardTo: searchField
        stickers: emoticonPickerTypeHeader.selectedType === EmojiPickerTypeHeader.EmoticonType.Sticker
        onStickerChosen: stickerModel.postSticker(emoticonFilterModel.mapToSource(emoticonFilterModel.index(index, 0)).row)
    }

    ImageContentModel {
        id: imageContentModel
        category: emoticonPickerCategoryHeader.currentCategory
    }

    Kirigami.Separator {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }

    QuickReaction {
        id: quickReaction
        onChosen: root.chosen(text)
        Layout.fillWidth: true
    }

    RecentImageContentProxyModel {
        id: recentImageContentProxyModel
    }

    ImageContentSearchModel {
        id: emojiSearchModel
        searchText: searchField.text
    }

    function clearSearchField() {
        searchField.text = ""
    }
}
