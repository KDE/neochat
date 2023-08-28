// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import QtWebView 1.15

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root

    FormCard.AbstractFormDelegate {
        background: null
        contentItem: WebView {
            id: webview
            url: "http://localhost:20847"
            implicitHeight: 500
            onLoadingChanged: {
                webview.runJavaScript("document.body.style.background = '" + Kirigami.Theme.backgroundColor + "'")
            }

            Timer {
                id: timer
                repeat: true
                running: true
                interval: 300
                onTriggered: {
                    if(!webview.visible) {
                        return
                    }
                    webview.runJavaScript("!!grecaptcha ? grecaptcha.getResponse() : \"\"", function(response){
                        if(!webview.visible || !response)
                            return
                        timer.running = false;
                        Registration.recaptchaResponse = response;
                    })
                }
            }
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/Username.qml")
    }
}
