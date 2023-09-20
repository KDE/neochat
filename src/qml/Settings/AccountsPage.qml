// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0
import 'Dialog' as Dialog

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
                onClicked: pageStack.layers.push("qrc:/AccountEditorPage.qml", {
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
                        Dialog.ConfirmLogout {
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
            onClicked: pageStack.layers.push("qrc:/WelcomePage.qml")
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
