// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root
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
            closeDialog()
        }
    }
}
