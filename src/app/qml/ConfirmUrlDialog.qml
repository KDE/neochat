// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: root

    property url link

    title: i18nc("@title:dialog", "Open URL")
    subtitle: xi18nc("@info", "Do you want to open <link>%1</link>?", root.link)
    dialogType: Kirigami.PromptDialog.Warning

    standardButtons: QQC2.DialogButtonBox.Open | QQC2.DialogButtonBox.Cancel

    onAccepted: Qt.openUrlExternally(root.link)
}
