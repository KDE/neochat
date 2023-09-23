// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQml

import org.kde.kirigami as Kirigami
import org.kde.neochat

Column {
    id: root

    property string emoji
    property string description

    QQC2.Label {
        id: emojiLabel
        x: 0
        y: 0
        width: parent.width
        height: parent.height * 0.75
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        text: root.emoji
        font.family: "emoji"
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 4
    }
    QQC2.Label {
        x: 0
        y: parent.height * 0.75
        width: parent.width
        height: parent.height * 0.25
        text: root.description
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
