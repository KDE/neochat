// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.neochat

ColumnLayout {
    id: root

    required property string icon
    required property string text

    anchors.fill: parent

    Item {
        Layout.fillHeight: true
    }
    Kirigami.Icon {
        Layout.fillWidth: true
        Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
        Layout.preferredHeight: Kirigami.Units.iconSizes.enormous
        source: root.icon
    }
    QQC2.Label {
        Layout.fillWidth: true
        text: root.text
        textFormat: Text.MarkdownText
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
    }
    Item {
        Layout.fillHeight: true
    }
}
