// SPDX-FileCopyrightText: 2022 Tobias Fella
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.neochat 1.0

QQC2.ScrollView {
    id: emojiGrid

    property alias model: emojis.model
    property alias count: emojis.count
    required property int targetIconSize
    readonly property int emojisPerRow: emojis.width / targetIconSize
    required property bool withCustom
    readonly property var searchCategory: withCustom ? EmojiModel.Search : EmojiModel.SearchNoCustom
    required property QtObject header
    property bool stickers: false

    signal chosen(string unicode)
    signal stickerChosen(int index)

    onActiveFocusChanged: if (activeFocus) {
        emojis.forceActiveFocus()
    }

    GridView {
        id: emojis

        anchors.fill: parent
        anchors.rightMargin: parent.QQC2.ScrollBar.vertical.visible ? parent.QQC2.ScrollBar.vertical.width : 0

        currentIndex: -1
        keyNavigationEnabled: true
        onActiveFocusChanged: if (activeFocus && currentIndex === -1) {
            currentIndex = 0
        } else {
            currentIndex = -1
        }
        onModelChanged: currentIndex = -1

        cellWidth: emojis.width / emojiGrid.emojisPerRow
        cellHeight: emojiGrid.targetIconSize

        KeyNavigation.up: emojiGrid.header

        clip: true

        delegate: EmojiDelegate {
            id: emojiDelegate
            checked: emojis.currentIndex === model.index
            emoji: !!modelData ? modelData.unicode : model.url
            name: !!modelData ? modelData.shortName : model.body

            width: emojis.cellWidth
            height: emojis.cellHeight

            isImage: emojiGrid.stickers
            Keys.onEnterPressed: clicked()
            Keys.onReturnPressed: clicked()
            onClicked: {
                if (emojiGrid.stickers) {
                    emojiGrid.stickerChosen(model.index)
                }
                emojiGrid.chosen(modelData.isCustom ? modelData.shortName : modelData.unicode)
                EmojiModel.emojiUsed(modelData)
            }
            Keys.onSpacePressed: pressAndHold()
            onPressAndHold: {
                if (EmojiModel.tones(modelData.shortName).length === 0) {
                    return;
                }
                let tones = tonesPopupComponent.createObject(emojiDelegate, {shortName: modelData.shortName, unicode: modelData.unicode, categoryIconSize: emojiGrid.targetIconSize})
                tones.open()
                tones.forceActiveFocus()
            }
            showTones: !!modelData && EmojiModel.tones(modelData.shortName).length > 0
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: emojiGrid.stickers ? i18n("No stickers") : i18n("No emojis")
            visible: emojis.count === 0
        }
    }
    Component {
        id: tonesPopupComponent
        EmojiTonesPicker {
            onChosen: emojiGrid.chosen(emoji)
        }
    }
}
