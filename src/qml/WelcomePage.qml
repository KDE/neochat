// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.neochat.accounts

FormCard.FormCardPage {
    id: root

    property bool showExisting: false
    property bool _showExisting: showExisting && module.source == root.initialStep
    property alias currentStep: module.item
    property string initialStep: "qrc:/org/kde/neochat/qml/LoginRegister.qml"

    signal connectionChosen

    title: i18n("Welcome")

    header: QQC2.Control {
        contentItem: Kirigami.InlineMessage {
            id: headerMessage
            type: Kirigami.MessageType.Error
            showCloseButton: true
            visible: false
        }
    }

    FormCard.FormCard {
        id: contentCard

        FormCard.AbstractFormDelegate {
            contentItem: Kirigami.Icon {
                source: "org.kde.neochat"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 16
            }
            background: Item {}
            onActiveFocusChanged: if (activeFocus) module.item.forceActiveFocus()
        }

        FormCard.FormTextDelegate {
            id: welcomeMessage
            text: i18n("Welcome to NeoChat")
        }
    }

    FormCard.FormHeader {
        id: existingAccountsHeader
        title: i18nc("@title", "Continue with an existing account")
        visible: (loadedAccounts.count > 0 || loadingAccounts.count > 0) && root._showExisting
    }

    FormCard.FormCard {
        visible: existingAccountsHeader.visible
        Repeater {
            id: loadedAccounts
            model: AccountRegistry
            delegate: FormCard.FormButtonDelegate {
                text: model.userId
                onClicked: {
                    Controller.activeConnection = model.connection
                    root.connectionChosen()
                }
            }
        }
        Repeater {
            id: loadingAccounts
            model: Controller.accountsLoading
            delegate: FormCard.FormButtonDelegate {
                text: i18nc("As in 'this account is still loading'", "%1 (loading)", modelData)
                enabled: false
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Log in or Create a New Account")
    }
    FormCard.FormCard {
        Loader {
            id: module
            Layout.fillWidth: true
            source: root.initialStep

            Connections {
                id: stepConnections
                target: currentStep

                function onProcessed(nextUrl) {
                    module.source = nextUrl;
                    headerMessage.text = "";
                    headerMessage.visible = false;
                    if (!module.item.noControls) {
                        module.item.forceActiveFocus()
                    } else {
                        continueButton.forceActiveFocus()
                    }
                }
                function onShowMessage(message) {
                    headerMessage.text = message;
                    headerMessage.visible = true;
                    headerMessage.type = Kirigami.MessageType.Information;
                }
                function onClearError() {
                    headerMessage.text = "";
                    headerMessage.visible = false;
                }
                function onCloseDialog() {
                    root.closeDialog();
                }
            }
            Connections {
                target: Registration
                function onNextStepChanged() {
                    if (Registration.nextStep === "m.login.recaptcha") {
                        stepConnections.onProcessed("qrc:/org/kde/neochat/qml/Captcha.qml")
                    }
                    if (Registration.nextStep === "m.login.terms") {
                        stepConnections.onProcessed("qrc:/org/kde/neochat/qml/Terms.qml")
                    }
                    if (Registration.nextStep === "m.login.email.identity") {
                        stepConnections.onProcessed("qrc:/org/kde/neochat/qml/Email.qml")
                    }
                    if (Registration.nextStep === "loading") {
                        stepConnections.onProcessed("qrc:/org/kde/neochat/qml/Loading.qml")
                    }
                }
            }
            Connections {
                target: LoginHelper
                function onErrorOccured(message) {
                    headerMessage.text = message;
                    headerMessage.visible = message.length > 0;
                    headerMessage.type = Kirigami.MessageType.Error;
                }
            }
        }

        FormCard.FormDelegateSeparator {
            below: continueButton
        }

        FormCard.FormButtonDelegate {
            id: continueButton
            text: root.currentStep.nextAction && root.currentStep.nextAction.text ? root.currentStep.nextAction.text : i18nc("@action:button", "Continue")
            visible: root.currentStep.nextAction
            onClicked: root.currentStep.nextAction.trigger()
            icon.name: "arrow-right"
            enabled: root.currentStep.nextAction ? root.currentStep.nextAction.enabled : false
        }

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Go back")
            visible: root.currentStep.previousAction
            onClicked: root.currentStep.previousAction.trigger()
            icon.name: "arrow-left"
            enabled: root.currentStep.previousAction ? root.currentStep.previousAction.enabled : false
        }
    }

    Component.onCompleted: {
        LoginHelper.init()
        module.item.forceActiveFocus()
        Registration.username = ""
        Registration.password = ""
        Registration.email = ""
    }
}
