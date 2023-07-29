// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.config

QQC2.Menu {
    id: root

    required property NeoChatConnection connection

    margins: Kirigami.Units.smallSpacing

    QQC2.MenuItem {
        text: i18n("Edit this account")
        icon.name: "document-edit"
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'AccountEditorPage.qml'), {
            connection: root.connection
        }, {
            title: i18n("Account editor")
        })
    }
    QQC2.MenuItem {
        text: i18n("Notification settings")
        icon.name: "notifications"
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'SettingsPage.qml'), {
            defaultPage: "notifications",
            connection: root.connection
        }, {
            title: i18n("Configure"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })
    }
    QQC2.MenuItem {
        text: i18n("Devices")
        icon.name: "computer-symbolic"
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'SettingsPage.qml'), {
            defaultPage: "devices",
            connection: root.connection
        }, {
            title: i18n("Configure"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })
    }
    QQC2.MenuItem {
        text: i18n("Open developer tools")
        icon.name: "tools"
        visible: Config.developerTools
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'DevtoolsPage.qml'), {
            connection: root.connection
        }, {
            title: i18nc("@title:window", "Developer Tools"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })
    }
    QQC2.MenuItem {
        text: i18nc("@action:inmenu", "Open Secret Backup")
        icon.name: "unlock"
        visible: Config.secretBackup
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UnlockSSSSDialog.qml'), {}, {
            title: i18nc("@title:window", "Open Key Backup")
        })
        enabled: Controller.ssssSupported
    }
    QQC2.MenuItem {
        text: i18n("Logout")
        icon.name: "list-remove-user"
        onTriggered: confirmLogoutDialogComponent.createObject(applicationWindow().overlay).open()
    }

    Component {
        id: confirmLogoutDialogComponent
        ConfirmLogoutDialog {
            connection: root.connection
        }
    }
}
