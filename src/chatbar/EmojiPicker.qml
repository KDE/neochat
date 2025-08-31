// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

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

    property bool includeCustom: false
    property bool showQuickReaction: false
    property bool showStickers: true

    readonly property var currentEmojiModel: {
        if (includeCustom) {
            EmojiModel.categoriesWithCustom;
        } else {
            EmojiModel.categories;
        }
    }

    readonly property int categoryIconSize: Math.round(Kirigami.Units.gridUnit * 2.5)
    readonly property var currentCategory: currentEmojiModel[categories.currentIndex].category
    readonly property alias categoryCount: categories.count
    property int selectedType: 0

    signal chosen(string emoji)

    onActiveFocusChanged: if (activeFocus) {
        searchField.forceActiveFocus();
    }

    spacing: 0

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

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: root.categoryIconSize + QQC2.ScrollBar.horizontal.height
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0
        visible: categories.count !== 0

        ListView {
            id: categories
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

            model: root.selectedType === 0 ? root.currentEmojiModel : stickerPackModel
            Component.onCompleted: categories.forceActiveFocus()

            delegate: root.selectedType === 0 ? emojiDelegate : stickerDelegate
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

    EmojiGrid {
        id: emojiGrid
        targetIconSize: root.currentCategory === EmojiModel.Custom ? Kirigami.Units.gridUnit * 3 : root.categoryIconSize  // Custom emojis are bigger
        model: root.selectedType === 1 ? emoticonFilterModel : searchField.text.length === 0 ? EmojiModel.emojis(root.currentCategory) : (root.includeCustom ? EmojiModel.filterModel(searchField.text, false) : EmojiModel.filterModelNoCustom(searchField.text, false))
        Layout.fillWidth: true
        Layout.fillHeight: true
        withCustom: root.includeCustom
        onChosen: unicode => root.chosen(unicode)
        header: categories
        Keys.forwardTo: searchField
        stickers: root.selectedType === 1
        onStickerChosen: index => stickerModel.postSticker(emoticonFilterModel.mapToSource(emoticonFilterModel.index(index, 0)).row)
    }

    Kirigami.Separator {
        visible: root.showQuickReaction
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }

    QQC2.ScrollView {
        visible: root.showQuickReaction
        Layout.fillWidth: true
        Layout.preferredHeight: root.categoryIconSize + QQC2.ScrollBar.horizontal.height
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0

        ListView {
            id: quickReactions
            Layout.fillWidth: true

            model: ["👍", "👎", "😄", "🎉", "😕", "❤", "🚀", "👀"]

            delegate: EmojiDelegate {
                required property string modelData
                emoji: modelData

                height: root.categoryIconSize
                width: height

                onClicked: root.chosen(modelData)
            }

            orientation: Qt.Horizontal
        }
    }

    ImagePacksModel {
        id: stickerPackModel
        room: root.currentRoom
        showStickers: true
        showEmoticons: false
    }

    StickerModel {
        id: stickerModel
        model: stickerPackModel
        packIndex: 0
        room: root.currentRoom
    }

    EmoticonFilterModel {
        id: emoticonFilterModel
        sourceModel: stickerModel
        showStickers: true
    }

    Component {
        id: emojiDelegate
        Kirigami.NavigationTabButton {
            required property string emoji
            required property int index
            required property string name
            width: root.categoryIconSize
            height: width
            checked: categories.currentIndex === index
            text: emoji
            QQC2.ToolTip.text: name
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
            onClicked: {
                categories.currentIndex = index;
                categories.focus = true;
            }
        }
    }

    Component {
        id: stickerDelegate
        Kirigami.NavigationTabButton {
            id: sticker
            required property string name
            required property int index
            required property string emoji
            width: root.categoryIconSize
            height: width
            checked: stickerModel.packIndex === index
            padding: Kirigami.Units.largeSpacing

            contentItem: Image {
                source: sticker.emoji
                fillMode: Image.PreserveAspectFit
                sourceSize.width: width
                sourceSize.height: height
            }
            QQC2.ToolTip.text: name
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered && !!name
            onClicked: stickerModel.packIndex = index
        }
    }

    function clearSearchField() {
        searchField.text = "";
    }
}
