// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami

import org.kde.neochat 1.0
import '../Dialog' as Dialog

QQC2.Menu {
    id: root
    margins: Kirigami.Units.smallSpacing

    QQC2.MenuItem {
        text: i18n("Edit this account")
        icon.name: "document-edit"
        onTriggered: pageStack.pushDialogLayer("qrc:/AccountEditorPage.qml", {
            connection: Controller.activeConnection
        }, {
            title: i18n("Account editor")
        });
    }
    QQC2.MenuItem {
        text: i18n("Notification settings")
        icon.name: "notifications"
        onTriggered: pageStack.pushDialogLayer("qrc:/SettingsPage.qml", {defaultPage: "notifications"}, { title: i18n("Configure")})
    }
    QQC2.MenuItem {
        text: i18n("Devices")
        icon.name: "computer-symbolic"
        onTriggered: pageStack.pushDialogLayer("qrc:/SettingsPage.qml", {defaultPage: "devices"}, { title: i18n("Configure")})
    }
    QQC2.MenuItem {
        text: i18n("Logout")
        icon.name: "list-remove-user"
        onTriggered: confirmLogoutDialog.open()
    }

    Dialog.ConfirmLogout {
        id: confirmLogoutDialog
    }
}
