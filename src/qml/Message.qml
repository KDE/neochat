// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQml

import org.kde.kirigami as Kirigami
import org.kde.neochat

Column {
    id: root

    required property string icon
    required property string text

    anchors.centerIn: parent
    Kirigami.Icon {
        width: Kirigami.Units.iconSizes.enormous
        height: width
        anchors.horizontalCenter: parent.horizontalCenter
        source: root.icon
    }
    QQC2.Label {
        text: root.text
        textFormat: Text.MarkdownText
        horizontalAlignment: Text.AlignHCenter
    }
}
