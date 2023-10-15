// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Dialogs

FileDialog {
    id: root

    signal chosen(string path)

    title: i18n("Select a File")
    onAccepted: root.chosen(selectedFile)
}
