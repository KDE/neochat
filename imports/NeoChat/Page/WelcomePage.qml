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

Kirigami.ScrollablePage {
    id: welcomePage

    property alias currentStep: module.item

    title: module.item.title ?? i18n("Welcome")

    header: Controls.Control {
        contentItem: Kirigami.InlineMessage {
            id: headerMessage
            type: Kirigami.MessageType.Error
            showCloseButton: true
            visible: false
        }
    }

    Component.onCompleted: LoginHelper.init()

    Connections {
        target: LoginHelper
        onErrorOccured: {
            headerMessage.text = message;
            headerMessage.visible = true;
            headerMessage.type = Kirigami.MessageType.Error;
        }
    }

    ColumnLayout {
        Kirigami.Icon {
            source: "org.kde.neochat"
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 16
        }
        Controls.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 25
            text: module.item.message ?? module.item.title ?? i18n("Welcome to Matrix")
        }

        Loader {
            id: module
            Layout.alignment: Qt.AlignHCenter
            source: "qrc:/imports/NeoChat/Component/Login/Login.qml"
            onSourceChanged: {
                headerMessage.visible = false
                headerMessage.text = ""
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            Controls.Button {
                text: i18nc("@action:button", "Back")

                enabled: welcomePage.currentStep.previousUrl !== ""
                visible: welcomePage.currentStep.showBackButton
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    module.source = welcomePage.currentStep.previousUrl
                }
            }

            Controls.Button {
                id: continueButton
                enabled: welcomePage.currentStep.acceptable
                visible: welcomePage.currentStep.showContinueButton
                action: welcomePage.currentStep.action
            }
        }

        Connections {
            target: currentStep

            function onProcessed(nextUrl) {
                module.source = nextUrl;
            }
            function onShowMessage(message) {
                headerMessage.text = message;
                headerMessage.visible = true;
                headerMessage.type = Kirigami.MessageType.Information;
            }
        }
    }
}
