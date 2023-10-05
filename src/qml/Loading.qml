// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    signal closeDialog

    FormCard.FormTextDelegate {
        text: i18n("Please wait. This might take a little while.")
    }
    FormCard.AbstractFormDelegate {
        contentItem: QQC2.BusyIndicator {}
        background: null
    }

    Connections {
        target: Controller
        function onInitiated() {
            root.closeDialog()
        }
    }
}
