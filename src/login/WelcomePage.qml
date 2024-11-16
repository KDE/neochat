// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.settings

Kirigami.Page {
    id: root

    property bool showExisting: false
    property bool _showExisting: showExisting && root.currentStepString === root.initialStep
    property alias currentStep: module.item
    property string currentStepString: initialStep
    property string initialStep: "LoginRegister"

    signal connectionChosen

    title: i18n("Welcome")
    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    header: QQC2.Control {
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0

        contentItem: ColumnLayout {
            spacing: 0

            Kirigami.Separator {
                Layout.fillWidth: true
            }

            Kirigami.InlineMessage {
                id: headerMessage
                type: Kirigami.MessageType.Error
                showCloseButton: true
                visible: false

                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing
            }
        }
    }

    contentItem: Item {
        ColumnLayout {
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            spacing: 0

            Kirigami.Icon {
                source: "org.kde.neochat"
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: Math.round(Kirigami.Units.iconSizes.huge * 1.5)
                implicitHeight: Math.round(Kirigami.Units.iconSizes.huge * 1.5)
            }

            Kirigami.Heading {
                id: welcomeMessage

                text: i18n("NeoChat")

                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Kirigami.Units.largeSpacing
            }

            FormCard.FormHeader {
                id: existingAccountsHeader
                title: i18nc("@title", "Continue with an existing account")
                visible: (loadedAccounts.count > 0 || loadingAccounts.count > 0) && root._showExisting
                maximumWidth: Kirigami.Units.gridUnit * 20
            }

            FormCard.FormCard {
                visible: existingAccountsHeader.visible
                maximumWidth: Kirigami.Units.gridUnit * 20
                Repeater {
                    id: loadedAccounts
                    model: AccountRegistry
                    delegate: FormCard.FormButtonDelegate {
                        id: delegate

                        required property string userId
                        required property NeoChatConnection connection

                        text: connection.localUser.displayName
                        description: connection.localUser.id
                        leadingPadding: Kirigami.Units.largeSpacing

                        onClicked: {
                            Controller.activeConnection = delegate.connection;
                            root.connectionChosen();
                        }
                        leading: KirigamiComponents.Avatar {
                            id: avatar
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: Kirigami.Units.gridUnit
                            name: delegate.text
                            // Note: User::avatarUrl does not set user_id, and thus cannot be used directly here. Hence the makeMediaUrl.
                            source: delegate.connection.localUser.avatarUrl.toString().length > 0 ? delegate.connection.makeMediaUrl(delegate.connection.localUser.avatarUrl) : ""
                            implicitWidth: Kirigami.Units.iconSizes.medium
                            implicitHeight: Kirigami.Units.iconSizes.medium
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
                                text: i18nc("@action:button", "Log out of this account")
                                icon.name: "im-kick-user"
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
                maximumWidth: Kirigami.Units.gridUnit * 20
            }

            FormCard.FormCard {
                maximumWidth: Kirigami.Units.gridUnit * 20
                Loader {
                    id: module
                    Layout.fillWidth: true
                    sourceComponent: Qt.createComponent('org.kde.neochat.login', root.initialStep)

                    Connections {
                        id: stepConnections
                        target: currentStep

                        function onProcessed(nextStep: string): void {
                            module.source = nextStep + ".qml";
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
                                stepConnections.onProcessed("Captcha");
                            }
                            if (Registration.nextStep === "m.login.terms") {
                                stepConnections.onProcessed("Terms");
                            }
                            if (Registration.nextStep === "m.login.email.identity") {
                                stepConnections.onProcessed("Email");
                            }
                            if (Registration.nextStep === "loading") {
                                stepConnections.onProcessed("Loading");
                            }
                        }
                    }
                    Connections {
                        target: LoginHelper

                        function onLoginErrorOccured(message) {
                            headerMessage.text = message;
                            headerMessage.visible = message.length > 0;
                            headerMessage.type = Kirigami.MessageType.Error;
                        }
                    }
                }

                FormCard.FormDelegateSeparator {
                    below: continueButton
                    visible: root.currentStep.nextAction
                }

                FormCard.FormButtonDelegate {
                    id: continueButton
                    text: root.currentStep.nextAction && root.currentStep.nextAction.text ? root.currentStep.nextAction.text : i18nc("@action:button", "Continue")
                    visible: root.currentStep.nextAction
                    onClicked: root.currentStep.nextAction.trigger()
                    icon.name: "arrow-right-symbolic"
                    enabled: root.currentStep.nextAction ? root.currentStep.nextAction.enabled : false
                }

                FormCard.FormButtonDelegate {
                    text: i18nc("@action:button", "Go back")
                    visible: root.currentStep.previousAction
                    onClicked: root.currentStep.previousAction.trigger()
                    icon.name: "arrow-left-symbolic"
                    enabled: root.currentStep.previousAction ? root.currentStep.previousAction.enabled : false
                }
            }

            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.largeSpacing * 2
                maximumWidth: Kirigami.Units.gridUnit * 20
                FormCard.FormButtonDelegate {
                    text: i18nc("@action:button", "Settings")
                    icon.name: "settings-configure"
                    onClicked: NeoChatSettingsView.open()
                }
            }
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
