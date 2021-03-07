/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0

import NeoChat.Component.Login 1.0

import QtWebView 1.15

LoginStep {
    id: captcha
    
    title: i18n("reCAPTCHA")
    
    headerVisible: false

    WebView {
        id: webview
        url: "http://localhost:20847"
        width: welcomePage.width
        height: welcomePage.height
        
        onLoadingChanged: {
            webview.runJavaScript("document.body.style.background = '" + Kirigami.Theme.backgroundColor + "'")
        }
        
        Timer {
            id: timer
            repeat: true
            running: true
            interval: 100
            onTriggered: {
                if(!webview.visible)
                    return
                webview.runJavaScript("grecaptcha.getResponse()", function(response){
                    if(!webview.visible || !response)
                        return
                    timer.running = false;
                    Registration.recaptResponse = response;
                    captcha.proceed("TODO");
                })
            }
        }
    }
}
