// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    title: i18nc("@title:window", "Keyboard Shortcuts")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Open Quick Switcher")
            description: "Ctrl+K"
        }

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Open Account Switcher")
            description: "Ctrl+U"
        }

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Search Messages in Current Room")
            description: "Ctrl+F"
        }

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Go to Previous Room")
            description: "Ctrl+PgUp, Ctrl+Backtab, Alt+Up"
        }

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Go to Next Room")
            description: "Ctrl+PgDown, Ctrl+Tab, Alt+Down"
        }

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Go to Previous Unread Room")
            description: "Alt+Shift+Up"
        }

        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Go to Next Unread Room")
            description: "Alt+Shift+Down"
        }
    }
}
