// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import org.kde.kirigami.actioncollection as AC
import org.kde.ki18n

AC.ActionCollectionManager {
    id: root

    AC.ActionCollection {
        name: "org.kde.neochat"
        text: KI18n.i18nc("@info", "NeoChat")

        AC.ActionData {
            name: "open_quickswitcher"
            text: KI18n.i18nc("@label", "Open Quick Switcher")
            defaultShortcut: "Ctrl+K"
            icon.name: "exchange-positions"
        }

        AC.ActionData {
            name: "open_accountswitcher"
            text: KI18n.i18nc("@label", "Open Account Switcher")
            defaultShortcut: "Ctrl+U"
            icon.name: "system-switch-user"
        }

        AC.ActionData {
            name: "search_messages"
            text: KI18n.i18nc("@label", "Search Messages in Current Room")
            defaultShortcut: "Ctrl+F"
            icon.name: "system-search"
        }

        AC.ActionData {
            name: "go_previous_room"
            text: KI18n.i18nc("@label", "Go to Previous Room")
            defaultShortcut: "Alt+Up"
            defaultAlternateShortcut: "Ctrl+Backtab"
            icon.name: "go-up"
        }

        AC.ActionData {
            name: "go_next_room"
            text: KI18n.i18nc("@label", "Go to Next Room")
            defaultShortcut: "Alt+Down"
            defaultAlternateShortcut: "Ctrl+Tab"
            icon.name: "go-down"
        }

        AC.ActionData {
            name: "go_previous_unread_room"
            text: KI18n.i18nc("@label", "Go to Previous Unread Room")
            defaultShortcut: "Alt+Shift+Up"
            icon.name: "go-up-skip"
        }

        AC.ActionData {
            name: "go_next_unread_room"
            text: KI18n.i18nc("@label", "Go to Next Unread Room")
            defaultShortcut: "Alt+Shift+Down"
                icon.name: "go-down-skip"
        }
    }
}
