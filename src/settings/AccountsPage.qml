// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatConnection initialAccount

    title: i18n("Accounts")

    Component.onCompleted: if (initialAccount) {
        intialAccountTimer.restart()
    }

    Timer {
        id: intialAccountTimer
        interval: 10
        running: false
        onTriggered: root.QQC2.ApplicationWindow.window.pageStack.layers.push(Qt.createComponent('org.kde.neochat.settings', 'AccountEditorPage'), {
            connection: initialAccount
        }, {
            title: i18n("Account editor")
        })
    }

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
                onClicked: root.QQC2.ApplicationWindow.window.pageStack.layers.push(Qt.createComponent('org.kde.neochat.settings', 'AccountEditorPage'), {
                    connection: accountDelegate.connection
                }, {
                    title: i18n("Account editor")
                })

                contentItem: RowLayout {
                    KirigamiComponents.Avatar {
                        name: accountDelegate.connection.localUser.displayName
                        // Note: User::avatarUrl does not set user_id, and thus cannot be used directly here. Hence the makeMediaUrl.
                        source: accountDelegate.connection.localUser.avatarUrl.toString().length > 0 ? accountDelegate.connection.makeMediaUrl(accountDelegate.connection.localUser.avatarUrl) : ""

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
                        onClicked: confirmLogoutDialogComponent.createObject(root.QQC2.Overlay.overlay).open()
                    }

                    Component {
                        id: confirmLogoutDialogComponent
                        ConfirmLogoutDialog {
                            connection: accountDelegate.connection
                            onAccepted: {
                                if (AccountRegistry.accountCount === 1) {
                                    root.Window.window.close();
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
        FormCard.FormDelegateSeparator {
            below: addAccountDelegate
        }

        FormCard.FormButtonDelegate {
            id: addAccountDelegate
            text: i18n("Add Account")
            icon.name: "list-add"
            onClicked: root.QQC2.ApplicationWindow.window.pageStack.layers.push(Qt.createComponent('org.kde.neochat.login', 'WelcomePage'), { showSettings: false })
        }
    }

    property Connections connections: Connections {
        target: Controller
        function onConnectionAdded() {
            if (pageStack.layers.depth > 2) {
                pageStack.layers.pop();
            }
        }
    }
}
