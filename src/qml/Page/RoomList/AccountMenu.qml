// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import '../Dialog' as Dialog

QQC2.Menu {
    id: root

    required property NeoChatConnection connection

    margins: Kirigami.Units.smallSpacing

    QQC2.MenuItem {
        text: i18n("Edit this account")
        icon.name: "document-edit"
        onTriggered: pageStack.pushDialogLayer("qrc:/AccountEditorPage.qml", {
            connection: root.connection
        }, {
            title: i18n("Account editor")
        });
    }
    QQC2.MenuItem {
        text: i18n("Notification settings")
        icon.name: "notifications"
        onTriggered: pageStack.pushDialogLayer("qrc:/SettingsPage.qml", {
            defaultPage: "notifications",
            connection: root.connection,
        }, {
            title: i18n("Configure")
        });
    }
    QQC2.MenuItem {
        text: i18n("Devices")
        icon.name: "computer-symbolic"
        onTriggered: pageStack.pushDialogLayer("qrc:/SettingsPage.qml", {
            defaultPage: "devices",
            connection: root.connection,
        }, {
            title: i18n("Configure")
        })
    }
    QQC2.MenuItem {
        text: i18n("Logout")
        icon.name: "list-remove-user"
        onTriggered: confirmLogoutDialogComponent.createObject(QQC2.ApplicationWindow.overlay).open()
    }

    Component {
        id: confirmLogoutDialogComponent
        Dialog.ConfirmLogout {
            connection: root.connection
        }
    }
}
