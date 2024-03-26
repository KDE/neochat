// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property NeoChatConnection connection

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title: dialog to switch between logged in accounts", "Account Switcher")

    onVisibleChanged: if (visible) {
        accountView.forceActiveFocus()
    }

    contentItem: AccountView {
        id: accountView
        connection: root.connection
        inDialog: true
    }
}
