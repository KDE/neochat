// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    FormCard.FormTextDelegate {
        id: keyField
        visible: false
    }

    Kirigami.LoadingPlaceholder {
        visible: !keyField.visible
        text: i18nc("@placeholder", "Initializing account")
    }

    Connections {
        target: Controlle.activeConnection
        function onBackupKeyCreated(key: String): void {
            keyField.text = key
            keyField.visible = true
        }
    }

    nextAction: Kirigami.Action {
        onTriggered: root.closeDialog()
        enabled: keyField.visible
    }
}
