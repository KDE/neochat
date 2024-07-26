// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Security")

    FormCard.FormHeader {
        title: i18nc("@title:group", "Invitations")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18nc("@option:check", "Reject invitations from unknown users")
            description: connection.canCheckMutualRooms ? i18n("If enabled, NeoChat will reject invitations from users you don't share a room with.") : i18n("Your server does not support this setting.")
            checked: Config.rejectUnknownInvites
            enabled: !Config.isRejectUnknownInvitesImmutable && connection.canCheckMutualRooms
            onToggled: {
                Config.rejectUnknownInvites = checked;
                Config.save();
            }
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title:group", "Ignored Users")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Manage ignored users")
            onClicked: root.ApplicationWindow.window.pageStack.push(ignoredUsersDialogComponent, {}, {
                title: i18nc("@title:window", "Ignored Users")
            });
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title", "Keys")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: connection.deviceKey
            description: i18n("Device key")
        }
        FormCard.FormTextDelegate {
            text: connection.encryptionKey
            description: i18n("Encryption key")
        }
        FormCard.FormTextDelegate {
            text: connection.deviceId
            description: i18n("Device id")
        }
    }

    Component {
        id: ignoredUsersDialogComponent
        IgnoredUsersDialog {
            connection: root.connection
        }
    }
}
