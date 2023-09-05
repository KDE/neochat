// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQml 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.neochat 1.0

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
    }
}
