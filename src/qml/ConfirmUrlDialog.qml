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

    title: i18nc("@title", "Open Url")
    standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.No

    contentItem: QQC2.Label {
        text: i18nc("Do you want to open <link>", "Do you want to open <b>%1</b>?", root.link)
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
