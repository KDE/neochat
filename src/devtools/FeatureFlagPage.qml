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
        text: i18nc("@option:check Enable the matrix feature to add a phone number as a third party ID", "Add phone numbers as 3PIDs")
        checked: NeoChatConfig.phone3PId

        onToggled: {
            NeoChatConfig.phone3PId = checked
            NeoChatConfig.save();
        }
    }
    FormCard.FormCheckDelegate {
        text: i18nc("@option:check Enable the matrix feature for audio and video calling", "Calls")
        checked: NeoChatConfig.calls

        onToggled: {
            NeoChatConfig.calls = checked;
            NeoChatConfig.save();
        }
    }
}
