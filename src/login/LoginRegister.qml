// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) {
        loginButton.forceActiveFocus(Qt.TabFocusReason);
    }

    Layout.fillWidth: true

    spacing: 0

    FormCard.FormButtonDelegate {
        id: loginButton
        icon.name: "user-symbolic"
        text: i18nc("@action:button", "Login")
        onClicked: root.processed("Login")
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormButtonDelegate {
        icon.name: "network-server-symbolic"
        text: i18nc("@action:button", "Register")
        onClicked: root.processed("Homeserver")
    }
}
