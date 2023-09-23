// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQml

import org.kde.neochat

Row {
    id: root

    property alias model: repeater.model
    anchors.horizontalCenter: parent.horizontalCenter
    Repeater {
        id: repeater
        delegate: EmojiItem {
            emoji: modelData.emoji
            description: modelData.description
            width: root.height
            height: width
        }
    }
}
