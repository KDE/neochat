// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtWebView

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root

    FormCard.AbstractFormDelegate {
        background: null
        contentItem: WebView {
            id: webview
            url: "http://localhost:20847"
            implicitHeight: 500
            onLoadingChanged: {
                webview.runJavaScript("document.body.style.background = '" + Kirigami.Theme.backgroundColor + "'");
            }

            Timer {
                id: timer
                repeat: true
                running: true
                interval: 300
                onTriggered: {
                    if (!webview.visible) {
                        return;
                    }
                    webview.runJavaScript("!!grecaptcha ? grecaptcha.getResponse() : \"\"", function (response) {
                        if (!webview.visible || !response)
                            return;
                        timer.running = false;
                        Registration.recaptchaResponse = response;
                    });
                }
            }
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("Username.qml")
    }
}
