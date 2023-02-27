// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

QQC2.Popup {
    id: tones

    signal chosen(string emoji)

    Component.onCompleted: {
        tonesList.currentIndex = 0;
        tonesList.forceActiveFocus();
    }

    required property string shortName
    required property string unicode
    required property int categoryIconSize
    width: tones.categoryIconSize * tonesList.count + 2 * padding
    height: tones.categoryIconSize + 2 * padding
    y: -height
    padding: 2
    modal: true
    dim: true
    onOpened: x = Math.min(parent.mapFromGlobal(QQC2.Overlay.overlay.width - tones.width, 0).x, -(width - parent.width) / 2)
    background: Kirigami.ShadowedRectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
        shadow.size: Kirigami.Units.smallSpacing
        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
        border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
        border.width: 1
    }

    ListView {
        id: tonesList
        width: parent.width
        height: parent.height
        orientation: Qt.Horizontal
        model: EmojiModel.tones(tones.shortName)
        keyNavigationEnabled: true
        keyNavigationWraps: true

        delegate: EmojiDelegate {
            id: emojiDelegate
            checked: tonesList.currentIndex === model.index
            emoji: modelData.unicode
            name: modelData.shortName

            width: tones.categoryIconSize
            height: width

            Keys.onEnterPressed: clicked()
            Keys.onReturnPressed: clicked()
            onClicked: {
                tones.chosen(modelData.unicode)
                EmojiModel.emojiUsed(modelData)
                tones.close()
            }
        }
    }
}
