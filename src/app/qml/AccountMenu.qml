// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.settings
import org.kde.neochat.devtools

KirigamiComponents.ConvergentContextMenu {
    id: root

    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow window

    QQC2.Action {
        text: i18nc("@action:button", "Show QR Code")
        icon.name: "view-barcode-qr-symbolic"
        onTriggered: {
            let qrMax = Qt.createComponent('org.kde.neochat', 'QrCodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                text: "https://matrix.to/#/" + root.connection.localUser.id,
                title: root.connection.localUser.displayName,
                subtitle: root.connection.localUser.id,
                // Note: User::avatarUrl does not set user_id, and thus cannot be used directly here. Hence the makeMediaUrl.
                avatarSource: root.connection.localUser.avatarUrl.toString().length > 0 ? root.connection.makeMediaUrl(root.connection.localUser.avatarUrl) : ""
            });
            if (typeof root.closeDialog === "function") {
                root.closeDialog();
            }
            qrMax.open();
        }
    }

    QQC2.Action {
        text: i18n("Edit This Account")
        icon.name: "document-edit"
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'AccountEditorPage'), {
            connection: root.connection
        }, {
            title: i18n("Account editor")
        })
    }

    QQC2.Action {
        text: i18n("Notification Settings")
        icon.name: "notifications"
        onTriggered: {
            NeoChatSettingsView.open('notifications');
        }
    }

    QQC2.Action {
        text: i18n("Devices")
        icon.name: "computer-symbolic"
        onTriggered: {
            NeoChatSettingsView.open('devices');
        }
    }

    Kirigami.Action {
        text: i18n("Open Developer Tools")
        icon.name: "tools"
        visible: NeoChatConfig.developerTools
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'DevtoolsPage'), {
            connection: root.connection
        }, {
            title: i18nc("@title:window", "Developer Tools"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Open Secret Backup")
        icon.name: "unlock"
        visible: NeoChatConfig.secretBackup
        onTriggered: root.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UnlockSSSSDialog'), {}, {
            title: i18nc("@title:window", "Open Key Backup")
        })
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Verify This Device")
        icon.name: "security-low"
        onTriggered: {
            root.connection.startSelfVerification();
            const dialog = Qt.createComponent("org.kde.kirigami", "PromptDialog").createObject(QQC2.Overlay.overlay, {
                title: i18nc("@title", "Verification Request Sent"),
                subtitle: i18nc("@info:label", "To proceed, accept the verification request on another device."),
                standardButtons: QQC2.Dialog.Ok
            })
            dialog.open();
            root.connection.onNewKeyVerificationSession.connect(() => {
                dialog.close();
            });
        }
    }

    QQC2.Action {
        text: i18n("Logout")
        icon.name: "im-kick-user"
        onTriggered: confirmLogoutDialogComponent.createObject(root).open()
    }

    readonly property Component confirmLogoutDialogComponent: ConfirmLogoutDialog {
        connection: root.connection
    }
}
