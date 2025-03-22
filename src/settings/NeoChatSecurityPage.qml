// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:window", "Security & Safety")

    header: Kirigami.InlineMessage {
        id: banner
        showCloseButton: true
        visible: false
        type: Kirigami.MessageType.Error
        position: Kirigami.InlineMessage.Position.Header
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4
        FormCard.FormButtonDelegate {
            id: ignoredUsersDelegate
            text: i18nc("@action:button", "Ignored Users")
            icon.name: "im-invisible-user"
            onClicked: root.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(ignoredUsersDialogComponent, {}, {
                title: i18nc("@title:window", "Ignored Users")
            });
        }
        FormCard.FormDelegateSeparator {
            above: ignoredUsersDelegate
            below: hideImagesDelegate
        }
        FormCard.FormCheckDelegate {
            id: hideImagesDelegate
            text: i18nc("@label:checkbox", "Hide images and videos by default")
            description: i18nc("@info", "When this option is enabled, images and videos are only shown after a button is clicked.")
            checked: NeoChatConfig.hideImages
            enabled: !NeoChatConfig.isHideImagesImmutable
            onToggled: {
                NeoChatConfig.hideImages = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: hideImagesDelegate
            below: rejectInvitationsDelegate
        }
        FormCard.FormCheckDelegate {
            id: rejectInvitationsDelegate
            text: i18nc("@option:check", "Reject invitations from unknown users")
            description: connection.canCheckMutualRooms ? i18nc("@info", "If enabled, NeoChat will reject invitations from users you don't share a room with.") : i18nc("@info", "Your server does not support this setting.")
            checked: NeoChatConfig.rejectUnknownInvites
            enabled: !NeoChatConfig.isRejectUnknownInvitesImmutable && connection.canCheckMutualRooms
            onToggled: {
                NeoChatConfig.rejectUnknownInvites = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: rejectInvitationsDelegate
            below: preferEncryptionDelegate
        }
        FormCard.FormCheckDelegate {
            id: preferEncryptionDelegate
            text: i18nc("@option:check", "Turn on encryption in new chats")
            description: i18nc("@info", "If enabled, NeoChat will use encryption when starting new direct messages.")
            checked: NeoChatConfig.preferUsingEncryption
            enabled: !NeoChatConfig.preferUsingEncryptionImmutable
            onToggled: {
                NeoChatConfig.preferUsingEncryption = checked;
                NeoChatConfig.save();
            }
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title:group", "Encryption")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            id: importKeysDelegate
            text: i18nc("@action:button", "Import Keys")
            description: i18nc("@info", "Import encryption keys from a backup.")
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
        FormCard.FormDelegateSeparator {
            above: importKeysDelegate
            below: exportKeysDelegate
        }
        FormCard.FormButtonDelegate {
            id: exportKeysDelegate
            text: i18nc("@action:button", "Export Keys")
            description: i18nc("@info", "Export this device's encryption keys.")
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
