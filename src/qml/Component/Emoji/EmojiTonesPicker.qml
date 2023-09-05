// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

QQC2.Popup {
    id: root

    signal chosen(string emoji)

    Component.onCompleted: {
        tonesList.currentIndex = 0;
        tonesList.forceActiveFocus();
    }

    required property string shortName
    required property string unicode
    required property int categoryIconSize
    width: root.categoryIconSize * tonesList.count + 2 * padding
    height: root.categoryIconSize + 2 * padding
    y: -height
    padding: 2
    modal: true
    dim: true
    clip: false
    onOpened: x = Math.min(parent.mapFromGlobal(QQC2.Overlay.overlay.width - root.width, 0).x, -(width - parent.width) / 2)
    background: Kirigami.ShadowedRectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.mediumSpacing
        shadow {
            size: Kirigami.Units.largeSpacing
            color: Qt.rgba(0.0, 0.0, 0.0, 0.3)
            yOffset: 2
        }
        border {
            color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
            width: 1
        }
    }

    ListView {
        id: tonesList
        width: parent.width
        height: parent.height
        orientation: Qt.Horizontal
        model: EmojiModel.tones(root.shortName)
        keyNavigationEnabled: true
        keyNavigationWraps: true

        delegate: EmojiDelegate {
            id: emojiDelegate
            checked: tonesList.currentIndex === model.index
            emoji: modelData.unicode
            name: modelData.shortName

            width: root.categoryIconSize
            height: width

            Keys.onEnterPressed: clicked()
            Keys.onReturnPressed: clicked()
            onClicked: {
                root.chosen(modelData.unicode)
                EmojiModel.emojiUsed(modelData)
                root.close()
            }
        }
    }
}
