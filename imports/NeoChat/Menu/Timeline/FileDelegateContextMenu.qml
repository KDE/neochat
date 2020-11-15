/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

import NeoChat.Dialog 1.0

Menu {
    signal viewSource()
    signal downloadAndOpen()
    signal saveFileAs()
    signal reply()
    signal redact()

    id: root

    MenuItem {
        text: i18n("View Source")

        onTriggered: viewSource()
    }

    MenuItem {
        text: i18n("Open Externally")

        onTriggered: downloadAndOpen()
    }

    MenuItem {
        text: i18n("Save As")

        onTriggered: saveFileAs()
    }

    MenuItem {
        text: i18n("Reply")

        onTriggered: reply()
    }

    MenuItem {
        text: i18n("Redact")

        onTriggered: redact()
    }

    onClosed: destroy()
}
