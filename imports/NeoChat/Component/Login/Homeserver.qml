// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

LoginStep {
    id: root

    readonly property string homeserver: customHomeserver.visible ? customHomeserver.text : serverCombo.currentText
    property bool loading: false
    showBackButton: true
    showContinueButton: true

    title: i18nc("@title", "Register")
    message: i18n("Select a Homeserver")
    previousUrl: "qrc:/imports/NeoChat/Component/Login/LoginRegister.qml"

    action: Kirigami.Action {
        text: Registration.testing ? i18n("Loading") : i18nc("@action:button", "Continue")
        enabled: Registration.homeserverAvailable && (!customHomeserver.visible || customHomeserver.acceptableInput)
        onTriggered: {
            processed("qrc:/imports/NeoChat/Component/Login/Username.qml");
        }
    }

    onHomeserverChanged: {
        Registration.homeserver = homeserver
    }

    Kirigami.FormLayout {
        Component.onCompleted: {
            Registration.homeserver = homeserver
        }

        QQC2.ComboBox {
            id: serverCombo

            Kirigami.FormData.label: i18n("Homeserver:")
            model: ["matrix.org", "kde.org", "tchncs.de", i18n("Other...")]
        }

        QQC2.TextField {
            id: customHomeserver

            Kirigami.FormData.label: i18n("Url:")
            visible: serverCombo.currentIndex === 3
            onTextChanged: {
                Registration.homeserver = text
            }
            validator: RegularExpressionValidator {
                regularExpression: /([a-zA-Z0-9\-]+\.)*[a-zA-Z0-9]+(:[0-9]+)?/
            }
        }
    }
}
