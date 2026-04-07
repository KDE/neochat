// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kemoji

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

    signal chosen(string emoji)

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
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2 + QQC2.ScrollBar.horizontal.height
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0
        Layout.alignment: Qt.AlignVCenter

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

            model: Dict.categories
            delegate: QQC2.ToolButton {
                required property var modelData
                display: QQC2.Button.IconOnly
                action: CategoryAction {
                    category: modelData
                }
                QQC2.ToolTip.text: action.text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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

            emojiPixelSize: Kirigami.Units.iconSizes.medium
            clip: true
            // onChosen: unicode => root.chosen(unicode)
        }
    }


    function clearSearchField() {
        searchField.text = "";
    }
}
