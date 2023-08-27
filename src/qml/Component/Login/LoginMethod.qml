// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

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
