// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kemoji as KEmoji

import org.kde.neochat

ColumnLayout {
    id: root

    readonly property alias count: emojiGrid.count

    readonly property alias model: emojiGrid.model

    readonly property int scrollBarWidth: grid.QQC2.ScrollBar.vertical.width
    readonly property int cellWidth: emojiGrid.cellWidth

    signal chosen(string emoji)

    function clearSearchField() {
        searchField.text = "";
    }

    onActiveFocusChanged: if (activeFocus) {
        searchField.forceActiveFocus();
    }

    spacing: 0

    Kirigami.SearchField {
        id: searchField
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.smallSpacing
        KeyNavigation.tab: emojiGrid
        onTextChanged: {
            emojiGrid.model.searchText = text

            // Always focus the first item if there is one
            if (emojiGrid.count === 0) {
                emojiGrid.currentIndex = -1;
            } else {
                emojiGrid.currentIndex = 0;
            }

            // If nothing was found, try again with all emojis
            if (emojiGrid.currentIndex < 0) {
                if (emojiGrid.model.currentCategory != KEmoji.Categories.All) {
                    emojiGrid.model.currentCategory = KEmoji.Categories.All;
                }
            }
        }
    }
    Kirigami.Separator {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }
    QQC2.ScrollView {
        id: grid
        Layout.fillWidth: true
        Layout.fillHeight: true
        EmojiGrid {
            id: emojiGrid

            font {
                family: "emoji"
                pixelSize: Kirigami.Units.iconSizes.smallMedium
            }
            clip: true

            onClicked: emoji => root.chosen(emoji.toString(Qt.RichText))
        }
    }
}
