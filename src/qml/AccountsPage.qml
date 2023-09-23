// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window
import Qt.labs.platform

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.accounts

FormCard.FormCardPage {
    id: root

    title: i18n("Accounts")

    FormCard.FormHeader {
        title: i18n("Accounts")
    }
    FormCard.FormCard {
        Repeater {
            model: AccountRegistry
            delegate: FormCard.AbstractFormDelegate {
                id: accountDelegate
                required property NeoChatConnection connection
                Layout.fillWidth: true
                onClicked: pageStack.layers.push("qrc:/org/kde/neochat/qml/AccountEditorPage.qml", {
                    connection: accountDelegate.connection
                }, {
                    title: i18n("Account editor")
                })

                contentItem: RowLayout {
                    KirigamiComponents.Avatar {
                        name: accountDelegate.connection.localUser.displayName
                        source: accountDelegate.connection.localUser.avatarMediaId ? ("image://mxc/" + accountDelegate.connection.localUser.avatarMediaId) : ""

                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        implicitWidth: Kirigami.Units.iconSizes.medium
                        implicitHeight: Kirigami.Units.iconSizes.medium
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: accountDelegate.connection.localUser.displayName
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            color: Kirigami.Theme.textColor
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: accountDelegate.connection.localUserId
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                    }

                    QQC2.ToolButton {
                        text: i18n("Logout")
                        icon.name: "im-kick-user"
                        onClicked: confirmLogoutDialogComponent.createObject(applicationWindow().overlay).open()
                    }

                    Component {
                        id: confirmLogoutDialogComponent
                        ConfirmLogoutDialog {
                            connection: model.connection
                            onAccepted: {
                                if (AccountRegistry.accountCount === 1) {
                                    root.Window.window.close()
                                }
                            }
                        }
                    }

                    FormCard.FormArrow {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        direction: Qt.RightArrow
                    }
                }
            }
        }
        FormCard.FormDelegateSeparator { below: addAccountDelegate }

        FormCard.FormButtonDelegate {
            id: addAccountDelegate
            text: i18n("Add Account")
            icon.name: "list-add"
            onClicked: pageStack.layers.push("qrc:/org/kde/neochat/qml/WelcomePage.qml")
        }
    }

    property Connections connections: Connections {
        target: Controller
        function onConnectionAdded() {
            if (pageStack.layers.depth > 2) {
                pageStack.layers.pop()
            }
        }
        function onPasswordStatus(status) {
            if (status === Controller.Success) {
                showPassiveNotification(i18n("Password changed successfully"));
            } else if (status === Controller.Wrong) {
                showPassiveNotification(i18n("Wrong password entered"));
            } else {
                showPassiveNotification(i18n("Unknown problem while trying to change password"));
            }
        }
    }
}
