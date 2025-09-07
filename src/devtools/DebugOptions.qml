// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCard {
    id: root

    Layout.topMargin: Kirigami.Units.largeSpacing

    FormCard.FormCheckDelegate {
        text: i18nc("@option:check", "Show hidden events in the timeline")
        checked: NeoChatConfig.showAllEvents

        onToggled: NeoChatConfig.showAllEvents = checked
    }
    FormCard.FormCheckDelegate {
        text: i18nc("@option:check", "Allow sending relations to any event in the timeline")
        description: i18nc("@info", "This includes state events")
        checked: NeoChatConfig.relateAnyEvent

        onToggled: NeoChatConfig.relateAnyEvent = checked
    }
    FormCard.FormCheckDelegate {
        id: roomAccountDataVisibleCheck
        text: i18nc("@option:check", "Always allow device verification")
        description: i18n("Allow the user to start a verification session with devices that were already verified")
        checked: NeoChatConfig.alwaysVerifyDevice

        onToggled: NeoChatConfig.alwaysVerifyDevice = checked
    }
    FormCard.FormCheckDelegate {
        text: i18nc("@option:check", "Show focus in window header")
        checked: NeoChatConfig.windowTitleFocus

        onToggled: {
            NeoChatConfig.windowTitleFocus = checked;
            NeoChatConfig.save();
        }
    }
}
