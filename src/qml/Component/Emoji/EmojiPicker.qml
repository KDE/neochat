// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

ColumnLayout {
    id: root

    property bool includeCustom: false

    readonly property var currentEmojiModel: {
        if (includeCustom) {
            EmojiModel.categoriesWithCustom
        } else {
            EmojiModel.categories
        }
    }

    readonly property int categoryIconSize: 45
    readonly property var currentCategory: currentEmojiModel[categories.currentIndex].category
    readonly property alias categoryCount: categories.count

    signal chosen(string emoji)

    spacing: 0

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: root.categoryIconSize + QQC2.ScrollBar.horizontal.height
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0

        ListView {
            id: categories
            clip: true
            focus: true
            orientation: ListView.Horizontal

            Keys.onReturnPressed: if (emojiGrid.count > 0) emojiGrid.focus = true
            Keys.onEnterPressed: if (emojiGrid.count > 0) emojiGrid.focus = true
            KeyNavigation.down: emojiGrid.count > 0 ? emojiGrid : categories
            KeyNavigation.tab: emojiGrid.count > 0 ? emojiGrid : categories

            keyNavigationEnabled: true
            keyNavigationWraps: true
            Keys.forwardTo: searchField
            interactive: width !== contentWidth

            model: root.currentEmojiModel
            Component.onCompleted: categories.forceActiveFocus()

            delegate: EmojiDelegate {
                width: root.categoryIconSize
                height: width

                checked: categories.currentIndex === model.index
                emoji: modelData.emoji
                name: modelData.name

                onClicked: {
                    categories.currentIndex = index
                    categories.focus = true
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
        targetIconSize: root.categoryIconSize
        model: searchField.text.length === 0 ? EmojiModel.emojis(root.currentCategory) : (root.includeCustom ? EmojiModel.filterModel(searchField.text, false) : EmojiModel.filterModelNoCustom(searchField.text, false))
        Layout.fillWidth: true
        Layout.fillHeight: true
        withCustom: root.includeCustom
        onChosen: root.chosen(unicode)
        header: categories
        Keys.forwardTo: searchField
    }
}
