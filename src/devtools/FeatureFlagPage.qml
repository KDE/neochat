// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat.config

FormCard.FormCardPage {
    id: root

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormCheckDelegate {
            id: roomAccountDataVisibleCheck
            text: i18nc("@option:check Enable the matrix 'threads' feature", "Threads")
            checked: Config.threads

            onToggled: Config.threads = checked
        }
        FormCard.FormCheckDelegate {
            text: i18nc("@option:check Enable the matrix 'secret backup' feature", "Secret Backup")
            checked: Config.secretBackup

            onToggled: Config.secretBackup = checked
        }
    }
}
