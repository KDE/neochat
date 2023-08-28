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
