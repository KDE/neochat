// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
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

    /**
     * @brief The current room that user is viewing.
     */
    property NeoChatRoom currentRoom

    property bool showStickers: true

    readonly property int scrollBarWidth: grid.QQC2.ScrollBar.vertical.width
    readonly property int cellWidth: emojiGrid.cellWidth
    readonly property int categoryIconSize: Math.round(Kirigami.Units.gridUnit * 2.5)

    signal chosen(string emoji)

    spacing: 0

    onActiveFocusChanged: if (activeFocus) {
        searchField.forceActiveFocus();
    }

    Kirigami.NavigationTabBar {
        id: types
        Layout.fillWidth: true
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        visible: root.showStickers

        background: null
        actions: [
            Kirigami.Action {
                id: emojis
                icon.name: "smiley"
                text: i18nc("@action:button", "Emojis")
                checked: true
                onTriggered: root.selectedType = 0
            },
            Kirigami.Action {
                id: stickers
                icon.name: "stickers"
                text: i18nc("@action:button", "Stickers")
                onTriggered: root.selectedType = 1
            }
        ]
    }

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2.5 + QQC2.ScrollBar.horizontal.height
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0
        Layout.alignment: Qt.AlignVCenter

        ListView {
            id: categories
            spacing: 0
            clip: true
            focus: true
            orientation: ListView.Horizontal

            Keys.onReturnPressed: if (emojiGrid.count > 0) {
                emojiGrid.focus = true;
            }
            Keys.onEnterPressed: if (emojiGrid.count > 0) {
                emojiGrid.focus = true;
            }

            KeyNavigation.down: emojiGrid.count > 0 ? emojiGrid : categories
            KeyNavigation.tab: emojiGrid.count > 0 ? emojiGrid : categories

            keyNavigationEnabled: true
            keyNavigationWraps: true
            Keys.forwardTo: searchField
            interactive: width !== contentWidth

            model: Dict.categories
            delegate: emojiCategoryDelegate
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
        visible: root.selectedType === 0

        /**
         * The focus is manged by the parent and we don't want to use the standard
         * shortcut as it could block other SearchFields from using it.
         */
        focusSequence: ""
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

    Component {
        id: emojiCategoryDelegate
        Kirigami.NavigationTabButton {
            required property var modelData
            KEmoji.Category.category: modelData
            width: root.categoryIconSize
            height: width
            display: QQC2.Button.IconOnly
            text: KEmoji.Category.name
            icon.name: KEmoji.Category.iconName
            checked: modelData === emojiGrid.model.currentCategory
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
            onClicked: emojiGrid.model.currentCategory = modelData
        }
    }


    function clearSearchField() {
        searchField.text = "";
    }
}
