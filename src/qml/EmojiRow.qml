// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

RowLayout {
    id: root

    property alias model: repeater.model

    spacing: Kirigami.Units.largeSpacing

    Repeater {
        id: repeater
        delegate: EmojiItem {
            emoji: modelData.emoji
            description: modelData.description
        }
    }
}
