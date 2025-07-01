// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components

import org.kde.neochat.libneochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:dialog", "Incoming Call")

    FormCard.FormCard {
        topPadding: Kirigami.Units.largeSpacing
        FormCard.AbstractFormDelegate {
            contentItem: Components.Avatar {
                name: CallManager.room.displayName
                source: CallManager.room.avatarMediaUrl
            }
        }
        FormCard.FormTextDelegate {
            text: i18nc("@info", "%1 is calling you.", CallManager.callingMember.htmlSafeDisplayName)
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Accept Call")
            onClicked: console.warn("unimplemented")
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Decline Call")
            onClicked: CallManager.declineCall()
        }
    }

}
