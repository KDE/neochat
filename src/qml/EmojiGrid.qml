// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.neochat

QQC2.ScrollView {
    id: root

    required property var model
    readonly property int emojisPerRow: emojis.width / Kirigami.Units.iconSizes.large
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

        cellWidth: emojis.width / root.emojisPerRow
        cellHeight: Kirigami.Units.iconSizes.large

        model: root.model

        KeyNavigation.up: root.header

        clip: true

        delegate: EmojiDelegate {
            id: emojiDelegate

            text: model.text
            checked: emojis.currentIndex === model.index
            toolTip: model.displayName

            width: emojis.cellWidth
            height: emojis.cellHeight

            Keys.onEnterPressed: clicked()
            Keys.onReturnPressed: clicked()
            onClicked: {
                root.chosen(model.isCustom ? (":" + model.shortCode + ":") : model.text)
            }
            Keys.onSpacePressed: pressAndHold()
            onPressAndHold: {
                if (EmojiModel.tones(model.displayName).length === 0) {
                    return;
                }
                let tones = tonesPopupComponent.createObject(emojiDelegate, {shortName: modelData.shortName, unicode: modelData.unicode, categoryIconSize: root.targetIconSize})
                tones.open()
                tones.forceActiveFocus()
            }
            showTones: false // TODO EmojiModel.tones(emojiDelegate.displayName).length > 0
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: root.stickers ? i18n("No stickers") : i18n("No emojis")
            visible: emojis.count === 0
        }
    }
    Component {
        id: tonesPopupComponent
        EmojiTonesPicker {
            onChosen: root.chosen(emoji)
        }
    }
}
