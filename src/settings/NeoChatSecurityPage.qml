// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Security")

    header: KirigamiComponents.Banner {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Invitations")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18nc("@option:check", "Reject invitations from unknown users")
            description: connection.canCheckMutualRooms ? i18n("If enabled, NeoChat will reject invitations from users you don't share a room with.") : i18n("Your server does not support this setting.")
            checked: NeoChatConfig.rejectUnknownInvites
            enabled: !NeoChatConfig.isRejectUnknownInvitesImmutable && connection.canCheckMutualRooms
            onToggled: {
                NeoChatConfig.rejectUnknownInvites = checked;
                NeoChatConfig.save();
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

    FormCard.FormHeader {
        visible: Controller.csSupported
        title: i18nc("@title", "Encryption Keys")
    }
    FormCard.FormCard {
        visible: Controller.csSupported
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Import Encryption Keys")
            icon.name: "document-import"
            onClicked: {
                let dialog = root.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat.settings", "ImportKeysDialog"), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Import Keys"),
                });
                dialog.success.connect(() => {
                    banner.text = i18nc("@info", "Keys imported successfully");
                    banner.type = Kirigami.MessageType.Positive;
                    banner.visible = true;
                });
                banner.visible = false;
            }
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Export Encryption Keys")
            icon.name: "document-export"
            onClicked: {
                root.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat.settings", "ExportKeysDialog"), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Export Keys")
                });
                banner.visible = false;
            }
        }
    }

    Component {
        id: ignoredUsersDialogComponent
        IgnoredUsersDialog {
            connection: root.connection
        }
    }
}
