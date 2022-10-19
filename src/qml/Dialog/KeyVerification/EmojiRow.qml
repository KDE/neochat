// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQml 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.neochat 1.0

Row {
    id: emojiRow

    property alias model: repeater.model
    anchors.horizontalCenter: parent.horizontalCenter
    Repeater {
        id: repeater
        delegate: EmojiItem {
            emoji: modelData.emoji
            description: modelData.description
            width: emojiRow.height
            height: width
        }
    }
}
