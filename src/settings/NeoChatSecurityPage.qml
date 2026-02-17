// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

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
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Messages")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: showLinkPreviewDelegate
            text: i18nc("@label:checkbox", "Show link previews")
            description: i18nc("@info:label", "You can customize this per-room under room settings. If unchecked, disables link previews in every room.")
            checked: NeoChatConfig.showLinkPreview
            onToggled: {
                NeoChatConfig.showLinkPreview = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: showLinkPreviewDelegate
            below: typingNotificationsDelegate
        }
        FormCard.FormCheckDelegate {
            id: typingNotificationsDelegate
            text: i18nc("@label:checkbox", "Send typing notifications")
            checked: NeoChatConfig.typingNotifications
            enabled: !NeoChatConfig.isTypingNotificationsImmutable
            onToggled: {
                NeoChatConfig.typingNotifications = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormCheckDelegate {
            id: publicReceiptsDelegate
            text: i18nc("@option:check", "Send read receipts")
            checked: NeoChatConfig.publicReadReceipts
            enabled: !NeoChatConfig.isPublicReadReceiptsImmutable
            onToggled: {
                NeoChatConfig.publicReadReceipts = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: publicReceiptsDelegate
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
    }
    FormCard.FormHeader {
        title: i18nc("@title:group", "Invites")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18nc("@info:label", "Everyone")
            checked: !NeoChatConfig.rejectUnknownInvites && !root.connection.blockAllInvites
            description: i18nc("@info:description", "Anyone can send you invites.")
        }

        FormCard.FormRadioDelegate {
            id: rejectInvitationsDelegate
            text: i18nc("@option:check", "Known users")
            description: root.connection.canCheckMutualRooms ? i18nc("@info", "Only users you share a room with can send you invites.") : i18nc("@info", "Your server does not support this setting.")
            checked: NeoChatConfig.rejectUnknownInvites
            enabled: !NeoChatConfig.isRejectUnknownInvitesImmutable && root.connection.canCheckMutualRooms
            onCheckedChanged: {
                NeoChatConfig.rejectUnknownInvites = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormRadioDelegate {
            text: i18nc("@info:label", "No one")
            checked: root.connection.blockAllInvites
            enabled: root.connection.supportsMatrixSpecVersion("v1.18")
            description: root.connection.supportsMatrixSpecVersion("v1.18") ? i18nc("@info:description", "No one can send you invites.") : i18nc("@info", "Your server does not support this setting.")
            onCheckedChanged: root.connection.blockAllInvites = checked
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title:group", "Encryption")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: preferEncryptionDelegate
            text: i18nc("@option:check", "Turn on encryption in new chats")
            description: i18nc("@info", "If enabled, NeoChat will use encryption when starting new direct messages.")
            checked: NeoChatConfig.preferUsingEncryption
            enabled: !NeoChatConfig.isPreferUsingEncryptionImmutable
            onToggled: {
                NeoChatConfig.preferUsingEncryption = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: preferEncryptionDelegate
            below: secretBackupDelegate
        }
        FormCard.FormButtonDelegate {
            id: secretBackupDelegate
            text: i18nc("@action:inmenu", "Manage Key Storage")
            description: i18nc("@info", "Import or unlock encryption keys from other devices.")
            icon.name: "unlock"
            onClicked: root.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UnlockSSSSDialog'), {}, {
                title: i18nc("@title:window", "Manage Secret Backup")
            })
        }
        FormCard.FormDelegateSeparator {
            above: secretBackupDelegate
            below: importKeysDelegate
        }
        FormCard.FormButtonDelegate {
            id: importKeysDelegate
            text: i18nc("@action:button", "Import Keys")
            description: i18nc("@info", "Import encryption keys from a backup file.")
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
            description: i18nc("@info", "Export this device's encryption keys to a file.")
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
