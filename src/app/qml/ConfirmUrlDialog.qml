// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.ki18n

Kirigami.PromptDialog {
    id: root

    property url link

    title: KI18n.i18nc("@title:dialog", "Open URL")
    subtitle: KI18n.xi18nc("@info", "Do you want to open <link>%1</link>?", root.link)
    dialogType: Kirigami.PromptDialog.Warning

    standardButtons: QQC2.DialogButtonBox.Open | QQC2.DialogButtonBox.Cancel

    onAccepted: Qt.openUrlExternally(root.link)
}
