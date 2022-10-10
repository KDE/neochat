// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami

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
        function onErrorOccured(message) {
            headerMessage.text = message;
            headerMessage.visible = true;
            headerMessage.type = Kirigami.MessageType.Error;
        }
    }

    Connections {
        target: Controller
        function onInitiated() {
            pageStack.layers.pop();
        }
    }

    property var showAvatar: LoginHelper.loginAvatar != ""

    ColumnLayout {
        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit * 10
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            id: swapper
            states: [
                State {
                    when: !LoginHelper.homeserverReachable
                    name: "idle"
                    PropertyChanges {
                        target: icon
                        opacity: 1
                    }
                    PropertyChanges {
                        target: avi
                        opacity: 0
                    }
                },
                State {
                    when: LoginHelper.homeserverReachable
                    name: "showAvi"
                    PropertyChanges {
                        target: icon
                        opacity: 0
                    }
                    PropertyChanges {
                        target: avi
                        opacity: 1
                    }
                }
            ]

            transitions: [
                Transition {
                    to: "showAvi"
                    SequentialAnimation {
                        NumberAnimation { target: icon; properties: "opacity";}
                        NumberAnimation { target: avi; properties: "opacity";}
                    }
                },
                Transition {
                    from: "showAvi"
                    SequentialAnimation {
                        NumberAnimation { target: avi; properties: "opacity";}
                        NumberAnimation { target: icon; properties: "opacity";}
                    }
                }
            ]

            Kirigami.Icon {
                id: icon
                source: "org.kde.neochat"
                anchors.fill: parent
                implicitWidth: height
            }

            ColumnLayout {
                id: avi
                opacity: 0
                anchors.fill: parent
                Kirigami.Avatar {
                    source: LoginHelper.loginAvatar
                    name: LoginHelper.loginName
                    Layout.fillHeight: true
                    implicitWidth: height
                    Layout.alignment: Qt.AlignHCenter
               }

               Controls.Label {
                   text: LoginHelper.loginName
                   font.pointSize: 24
                   Layout.alignment: Qt.AlignHCenter
               }
            }
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
                opacity: welcomePage.currentStep.showContinueButton ? 1 : 0
                Behavior on opacity { NumberAnimation {} }
                action: welcomePage.currentStep.action
            }
        }

        Kirigami.LoadingPlaceholder {
            opacity: LoginHelper.testing ? 1 : 0
            text: i18n("Connecting to your homeserver...")
            Behavior on opacity { NumberAnimation {} }
            Layout.alignment: Qt.AlignHCenter
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
