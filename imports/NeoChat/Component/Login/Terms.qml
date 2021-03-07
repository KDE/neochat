/**
 * SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

LoginStep {
    id: terms

    showContinueButton: true
    showBackButton: true

    title: i18nc("@title", "Registration")
    message: i18n("Terms and Conditions")
    //previousUrl: "qrc:/imports/NeoChat/Component/Login/Homeserver.qml"

    Kirigami.FormLayout {
        
    }

    action: Kirigami.Action {
        text: Registration.testingUsername ? i18n("Loading")
                : Registration.usernameAvailable ? i18nc("@action:button", "Continue")
                : i18n("Username unavailable")
        onTriggered: {
            console.log("todoooo")
        }
        enabled: Registration.usernameAvailable
        
    }
}
