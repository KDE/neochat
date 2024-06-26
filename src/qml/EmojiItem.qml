// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.neochat

ColumnLayout {
    id: root

    property alias emoji: emojiLabel.text
    property alias description: descriptionLabel.text

    QQC2.Label {
        id: emojiLabel
        Layout.fillWidth: true
        Layout.preferredWidth: Kirigami.Units.iconSizes.huge
        Layout.preferredHeight: Kirigami.Units.iconSizes.huge
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.family: "emoji"
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 4
    }
    QQC2.Label {
        id: descriptionLabel
        Layout.fillWidth: true
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
