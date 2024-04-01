// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: root

    property url link

    width: Kirigami.Units.gridUnit * 24
    height: Kirigami.Units.gridUnit * 8

    title: i18nc("@title", "Open URL")
    standardButtons: QQC2.DialogButtonBox.Open | QQC2.DialogButtonBox.Cancel

    contentItem: QQC2.Label {
        text: xi18nc("@info", "Do you want to open <link>%1</link>?", root.link)
        wrapMode: QQC2.Label.Wrap
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }

    onAccepted: {
        Qt.openUrlExternally(root.link);
        root.close();
    }
    onRejected: {
        root.close();
    }
}
