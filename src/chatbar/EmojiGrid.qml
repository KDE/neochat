// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.textaddons.emoticons

QQC2.ScrollView {
    id: root

    readonly property int emojisPerRow: emojis.width / Kirigami.Units.iconSizes.large
    required property QtObject header
    property bool stickers: false

    signal chosen(string unicode)
    signal stickerChosen(int index)

    onActiveFocusChanged: if (activeFocus) {
        emojis.forceActiveFocus();
    }

    width: Kirigami.Units.gridUnit * 24

    GridView {
        id: emojis

        anchors.fill: parent
        anchors.rightMargin: parent.QQC2.ScrollBar.vertical.visible ? parent.QQC2.ScrollBar.vertical.width : 0

        currentIndex: -1
        keyNavigationEnabled: true
        onActiveFocusChanged: if (activeFocus && currentIndex === -1) {
            currentIndex = 0;
        } else {
            currentIndex = -1;
        }
        onModelChanged: currentIndex = -1

        cellWidth: emojis.width / root.emojisPerRow
        cellHeight: Kirigami.Units.iconSizes.large

        model: EmojiModelManager.emojiModel

        KeyNavigation.up: root.header

        clip: true

        delegate: EmojiDelegate {
            id: emojiDelegate

            required property string unicode
            required property string identifier
            required property int index

            text: emojiDelegate.unicode
            toolTip: emojiDelegate.identifier
            checked: emojis.currentIndex === emojiDelegate.index

            width: emojis.cellWidth
            height: emojis.cellHeight

            Keys.onEnterPressed: clicked()
            Keys.onReturnPressed: clicked()
            // onClicked: {
            //     if (root.stickers) {
            //         root.stickerChosen(model.index);
            //     }
            //     root.chosen(modelData.isCustom ? modelData.shortName : modelData.unicode);
            //     EmojiModel.emojiUsed(modelData);
            // }
            // Keys.onSpacePressed: pressAndHold()
            // onPressAndHold: {
            //     if (!showTones) {
            //         return;
            //     }
            //     let tones = Qt.createComponent("org.kde.neochat", "EmojiTonesPicker").createObject(emojiDelegate, {
            //         shortName: modelData.shortName,
            //         unicode: modelData.unicode,
            //         categoryIconSize: root.targetIconSize,
            //         onChosen: root.chosen(emoji => root.chosen(emoji))
            //     });
            //     tones.open();
            //     tones.forceActiveFocus();
            // }
            // showTones: model.hasTones
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            icon.name: root.stickers ? "stickers" : "preferences-desktop-emoticons"
            text: root.stickers ? i18nc("@info", "No stickers") : i18nc("@info", "No emojis")
            visible: emojis.count === 0
        }
    }
}
