// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

LoginStep {
    id: root

    readonly property var homeserver: customHomeserver.visible ? customHomeserver.text : serverCombo.currentText
    property bool loading: false

    title: i18n("@title", "Select a Homeserver")

    action: Kirigami.Action {
        enabled: LoginHelper.homeserverReachable && !customHomeserver.visible || customHomeserver.acceptableInput
        onTriggered: {
            // TODO
            console.log("register todo")
        }
    }

    onHomeserverChanged: {
        LoginHelper.testHomeserver("@user:" + homeserver)
    }

    Kirigami.FormLayout {
        Component.onCompleted: Controller.testHomeserver(homeserver)

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
                Controller.testHomeserver(text)
            }
            validator: RegularExpressionValidator {
                regularExpression: /([a-zA-Z0-9\-]+\.)*[a-zA-Z0-9]+(:[0-9]+)?/
            }
        }

        QQC2.Button {
            id: continueButton
            text: i18nc("@action:button", "Continue")
            action: root.action
        }
    }
}
