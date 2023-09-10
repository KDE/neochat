// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) loginButton.forceActiveFocus()

    Layout.fillWidth: true

    FormCard.FormButtonDelegate {
        id: loginButton
        text: i18nc("@action:button", "Login")
        onClicked: root.processed("qrc:/Login.qml")
    }

    FormCard.FormButtonDelegate {
        text: i18nc("@action:button", "Register")
        onClicked: root.processed("qrc:/Homeserver.qml")
    }
}
