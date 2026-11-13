// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Dialogs
import org.kde.ki18n

FileDialog {
    id: root

    signal chosen(string path)

    title: KI18n.i18nc("@title:dialog", "Select a File")
    onAccepted: root.chosen(selectedFile)
}
