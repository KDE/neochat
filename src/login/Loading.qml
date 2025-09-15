// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    FormCard.FormTextDelegate {
        textItem.wrapMode: Text.Wrap
        text: i18n("Please wait while your messages are loaded from the server. This might take a little while.")
    }
    FormCard.AbstractFormDelegate {
        contentItem: QQC2.BusyIndicator {}
        background: null
    }

    Connections {
        target: Controller
        function onConnectionAdded(connection) {
            connection.syncDone.connect(() => root.closeDialog());
        }
    }
}
