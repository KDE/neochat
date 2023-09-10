// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import Qt.labs.platform

FileDialog {
    signal chosen(string path)

    id: root

    title: i18n("Please choose a file")

    onAccepted: chosen(file)
}
