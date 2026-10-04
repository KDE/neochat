// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kemoji as KEmoji

import org.kde.neochat

QQC2.Popup {
    id: root

    property list<StickerList> stickerLists: []

    property bool closeOnChosen: true
    property bool showStickers: true

    property int selectedType: 0

    readonly property int categoryIconSize: Math.round(Kirigami.Units.gridUnit * 2.5)

    signal chosen(string emoji)

    signal stickerChosen(packIndex: int, stickerIndex: int)

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            root.close();
        }
    }

    onVisibleChanged: {
        if (!visible) {
            (gridLoader.item as EmojiCategoryGrid).clearSearchField();
            return;
        }
        (gridLoader.item as EmojiCategoryGrid).forceActiveFocus();
    }

    background: Kirigami.ShadowedRectangle {
        radius: Kirigami.Units.cornerRadius
        color: Kirigami.Theme.backgroundColor

        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        }

        shadow {
            size: Kirigami.Units.gridUnit
            yOffset: 0
            color: Qt.rgba(0, 0, 0, 0.2)
        }

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
    }

    modal: true
    focus: true
    clip: false
    closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutsideParent
    margins: 0
    padding: 2

    implicitHeight: Kirigami.Units.gridUnit * 20 + 2 * padding
    width: Kirigami.Units.gridUnit * 30 + 2 * padding
    contentItem: ColumnLayout {
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

                interactive: width !== contentWidth

                model: root.selectedType === 0 ? Dict.categories : root.stickerLists.length
                delegate: root.selectedType === 0 ? emojiCategoryDelegate : stickerCategoryDelegate
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }
        Loader {
            id: gridLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            sourceComponent: root.selectedType === 0 ? emojiGrid : stickerGrid
        }
    }

    Component {
        id: emojiGrid
        EmojiCategoryGrid {
            onChosen: emoji => {
                root.chosen(emoji);
                if (root.closeOnChosen) {
                    root.close();
                }
            }
        }
    }

    Component {
        id: stickerGrid
        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            onActiveFocusChanged: if (activeFocus) {
                grid.forceActiveFocus();
            }
            GridView {
                id: grid
                cellWidth: 75
                cellHeight: cellWidth
                clip: true
                focus: true
                model: StickerModel {
                    id: stickerModel
                    stickerLists: root.stickerLists
                }
                delegate: QQC2.ItemDelegate {
                    id: stickerDelegate
                    required property int index
                    required property url source
                    required property string name

                    width: grid.cellWidth
                    height: grid.cellHeight
                    text: name
                    display: QQC2.Button.IconOnly
                    contentItem: Image {
                        source: stickerDelegate.source
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered

                    onClicked: {
                        root.stickerChosen(stickerModel.listIndex, index)
                        root.close();
                    }
                }
            }
        }
    }

    Component {
        id: emojiCategoryDelegate
        Kirigami.NavigationTabButton {
            required property var modelData
            KEmoji.Category.category: modelData

            Keys.onReturnPressed: click()
            Keys.onEnterPressed: click()

            width: root.categoryIconSize
            height: width
            display: QQC2.Button.IconOnly
            text: KEmoji.Category.name
            icon.name: KEmoji.Category.iconName
            checked: modelData === (gridLoader.item as EmojiCategoryGrid)?.model.currentCategory ?? KEmoji.Categories.None
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
            onClicked: (gridLoader.item as EmojiCategoryGrid).model.currentCategory = modelData
        }
    }

    Component {
        id: stickerCategoryDelegate
        Kirigami.NavigationTabButton {
            id: stickerButton
            required property int index
            width: root.categoryIconSize
            height: width
            padding: Kirigami.Units.largeSpacing
            text: root.stickerLists[stickerButton.index]?.name ?? ""
            checked: ((((gridLoader.item as QQC2.ScrollView)?.contentItem as GridView)?.model as StickerModel)?.listIndex ?? -1) === index

            contentItem: Image {
                source: root.stickerLists[stickerButton.index]?.source ?? ""
                fillMode: Image.PreserveAspectFit
                sourceSize.width: width
                sourceSize.height: height
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered && !!text
            onClicked: (((gridLoader.item as QQC2.ScrollView).contentItem as GridView).model as StickerModel).listIndex = index
        }
    }
}
