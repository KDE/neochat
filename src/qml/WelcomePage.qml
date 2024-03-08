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
    property bool _showExisting: showExisting && root.currentStepString === root.initialStep
    property alias currentStep: module.item
    property string currentStepString: initialStep
    property string initialStep: "LoginRegister.qml"

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

    Kirigami.Icon {
        source: "org.kde.neochat"
        Layout.alignment: Qt.AlignHCenter
        implicitWidth: Math.round(Kirigami.Units.iconSizes.huge * 1.5)
        implicitHeight: Math.round(Kirigami.Units.iconSizes.huge * 1.5)
    }

    Kirigami.Heading {
        id: welcomeMessage

        text: i18n("Welcome to NeoChat")

        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: Kirigami.Units.largeSpacing
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
                    Controller.activeConnection = model.connection;
                    root.connectionChosen();
                }
            }
        }
        Repeater {
            id: loadingAccounts
            model: Controller.accountsLoading
            delegate: FormCard.AbstractFormDelegate {
                id: loadingDelegate

                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing

                background: null
                contentItem: RowLayout {
                    spacing: 0

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: i18nc("As in 'this account is still loading'", "%1 (loading)", modelData)
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        color: Kirigami.Theme.disabledTextColor
                        Accessible.ignored: true // base class sets this text on root already
                    }

                    QQC2.ToolButton {
                        text: i18nc("@action:button", "Remove this account")
                        icon.name: "edit-delete-remove"
                        onClicked: Controller.removeConnection(modelData)
                        display: QQC2.Button.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        enabled: true
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                    }

                    FormCard.FormArrow {
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        direction: Qt.RightArrow
                        visible: root.background.visible
                    }
                }
            }
            onCountChanged: {
                if (loadingAccounts.count === 0 && loadedAccounts.count === 1 && showExisting) {
                    Controller.activeConnection = AccountRegistry.data(AccountRegistry.index(0, 0), 257);
                    root.connectionChosen();
                }
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
            sourceComponent: Qt.createComponent('org.kde.neochat', root.initialStep)

            Connections {
                id: stepConnections
                target: currentStep

                function onProcessed(nextStep: string): void {
                    module.source = nextStep;
                    root.currentStepString = nextStep;
                    headerMessage.text = "";
                    headerMessage.visible = false;
                    if (!module.item.noControls) {
                        module.item.forceActiveFocus();
                    } else {
                        continueButton.forceActiveFocus();
                    }
                }

                function onShowMessage(message: string): void {
                    headerMessage.text = message;
                    headerMessage.visible = true;
                    headerMessage.type = Kirigami.MessageType.Information;
                }

                function onClearError(): void {
                    headerMessage.text = "";
                    headerMessage.visible = false;
                }

                function onCloseDialog(): void {
                    root.closeDialog();
                }
            }

            Connections {
                target: Registration
                function onNextStepChanged() {
                    if (Registration.nextStep === "m.login.recaptcha") {
                        stepConnections.onProcessed("Captcha.qml");
                    }
                    if (Registration.nextStep === "m.login.terms") {
                        stepConnections.onProcessed("Terms.qml");
                    }
                    if (Registration.nextStep === "m.login.email.identity") {
                        stepConnections.onProcessed("Email.qml");
                    }
                    if (Registration.nextStep === "loading") {
                        stepConnections.onProcessed("Loading.qml");
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

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Open proxy settings")
            icon.name: "settings-configure"
            onClicked: pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat", "NetworkProxyPage.qml"), {}, {
                title: i18nc("@title:window", "Proxy Settings")
            });
        }
    }

    Component.onCompleted: {
        LoginHelper.init();
        module.item.forceActiveFocus();
        Registration.username = "";
        Registration.password = "";
        Registration.email = "";
    }
}
