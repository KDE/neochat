// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

QQC2.ScrollView {
    id: root

    signal chosen(string text)

    QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0

    implicitHeight: Kirigami.Units.iconSizes.large + QQC2.ScrollBar.horizontal.height

    ListView {
        id: quickReactions
        Layout.fillWidth: true

        model: ["👍", "👎", "😄", "🎉", "😕", "❤", "🚀", "👀"]

        delegate: EmojiDelegate {
            height: Kirigami.Units.iconSizes.large
            width: height

            text: modelData
            onClicked: root.chosen(modelData)
        }

        orientation: Qt.Horizontal
    }
}
