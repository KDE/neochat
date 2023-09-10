// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    onActiveFocusChanged: if (activeFocus) loginPasswordButton.forceActiveFocus()

    FormCard.FormButtonDelegate {
        id: loginPasswordButton
        text: i18nc("@action:button", "Login with password")
        onClicked: processed("qrc:/Password.qml")
    }

    FormCard.FormButtonDelegate {
        id: loginSsoButton
        text: i18nc("@action:button", "Login with single sign-on")
        onClicked: processed("qrc:/Sso.qml")
    }
}
