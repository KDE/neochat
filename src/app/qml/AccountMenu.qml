// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtMultimedia

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.settings

KirigamiComponents.ConvergentContextMenu {
    id: root

    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow window

    data: MediaDevices {
        id: devices
    }

    Kirigami.Action {
        text: i18nc("@action:button", "Open Profile")
        icon.name: "im-user-symbolic"
        onTriggered: RoomManager.resolveResource(root.connection.localUserId, "qr") // Use "qr" action to make sure a room isn't passed, see RoomManager::visitUser
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Scan a QR Code")
        icon.name: "document-scan-symbolic"
        visible: devices.videoInputs.length > 0
        onTriggered: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent("org.kde.neochat", "QrScannerPage"), {
            connection: root.connection
        }, {
            title: i18nc("@title", "Scan a QR Code")
        })
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Notification Settings")
        icon.name: "notifications"
        onTriggered: {
            NeoChatSettingsView.open('notifications');
        }
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Open Developer Tools")
        icon.name: "tools"
        visible: NeoChatConfig.developerTools
        onTriggered: root.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'DevtoolsPage'), {
            connection: root.connection
        }, {
            title: i18nc("@title:window", "Developer Tools"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Verify This Device")
        icon.name: "security-low"
        visible: !root.connection.isVerifiedSession
        onTriggered: {
            root.connection.startSelfVerification();
            const dialog = Qt.createComponent("org.kde.kirigami", "PromptDialog").createObject(QQC2.Overlay.overlay, {
                title: i18nc("@title", "Verification Request Sent"),
                subtitle: i18nc("@info:label", "To proceed, accept the verification request on another device."),
                standardButtons: Kirigami.Dialog.Ok
            })
            dialog.open();
            root.connection.newKeyVerificationSession.connect(() => {
                dialog.close();
            });
        }
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu Open support dialog", "Support")
        icon.name: "help-contents-symbolic"
        onTriggered: {
            (Qt.createComponent("org.kde.neochat", "SupportDialog").createObject(QQC2.Overlay.overlay, {
                connection: root.connection,
            }) as SupportDialog).open();
        }
    }

    Kirigami.Action {
        separator: true
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Switch Account")
        icon.name: "system-switch-user"
        shortcut: "Ctrl+U"
        onTriggered: (Qt.createComponent("org.kde.neochat", "AccountSwitchDialog").createObject(QQC2.Overlay.overlay, {
            connection: root.connection
        }) as Kirigami.Dialog).open();
    }
}
