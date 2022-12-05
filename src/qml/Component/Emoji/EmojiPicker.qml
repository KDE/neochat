// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

ColumnLayout {
    id: emojiPicker

    readonly property int categoryIconSize: 45
    readonly property var currentCategory: EmojiModel.categoriesWithCustom[categories.currentIndex].category
    readonly property int categoryCount: categories.count

    signal chosen(string emoji)

    spacing: 0

    onActiveFocusChanged: if (activeFocus) categories.forceActiveFocus()

    QQC2.ScrollView {
        Layout.fillWidth: true
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0
        Layout.preferredHeight: emojiPicker.categoryIconSize + QQC2.ScrollBar.horizontal.height

        ListView {
            id: categories
            clip: true
            orientation: ListView.Horizontal

            keyNavigationEnabled: true
            keyNavigationWraps: true
            Keys.forwardTo: searchField
            interactive: width !== contentWidth

            model: EmojiModel.categoriesWithCustom
            delegate: EmojiDelegate {
                id: emojiDelegate

                width: emojiPicker.categoryIconSize
                height: width

                checked: categories.currentIndex === model.index
                emoji: modelData.emoji
                name: modelData.name

                onClicked: {
                    categories.currentIndex = index
                }
            }
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
    }

    EmojiGrid {
        id: emojiGrid
        targetIconSize: emojiPicker.categoryIconSize
        model: searchField.text.length === 0 ? EmojiModel.emojis(emojiPicker.currentCategory) : EmojiModel.filterModel(searchField.text, false)
        Layout.fillWidth: true
        Layout.preferredHeight: 350
        onChosen: emojiPicker.chosen(unicode)
        withCustom: true
        header: categories

        Keys.forwardTo: searchField
    }
}
