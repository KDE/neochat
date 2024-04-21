// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormCheckDelegate {
            text: i18nc("@option:check", "Show hidden events in the timeline")
            checked: Config.showAllEvents

            onToggled: Config.showAllEvents = checked
        }
        FormCard.FormCheckDelegate {
            id: roomAccountDataVisibleCheck
            text: i18nc("@option:check Enable the matrix 'threads' feature", "Always allow device verification")
            description: i18n("Allow the user to start a verification session with devices that were already verified")
            checked: Config.alwaysVerifyDevice

            onToggled: Config.alwaysVerifyDevice = checked
        }
        FormCard.FormCheckDelegate {
            text: i18nc("@option:check", "Show focus in window header")
            checked: Config.windowTitleFocus

            onToggled: {
                Config.windowTitleFocus = checked;
                Config.save();
            }
        }
    }
}
