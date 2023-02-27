// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Accounts")
    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Accounts")
                }

                Repeater {
                    model: AccountRegistry
                    delegate: MobileForm.AbstractFormDelegate {
                        Layout.fillWidth: true
                        onClicked: pageSettingStack.pushDialogLayer("qrc:/AccountEditorPage.qml", {
                            connection: model.connection
                        }, {
                            title: i18n("Account editor")
                        })

                        contentItem: RowLayout {
                            Kirigami.Avatar {
                                name: model.connection.localUser.displayName ?? model.connection.localUser.id
                                source: model.connection.localUser.avatarMediaId ? ("image://mxc/" + model.connection.localUser.avatarMediaId) : ""

                                Layout.rightMargin: Kirigami.Units.largeSpacing
                                implicitWidth: Kirigami.Units.iconSizes.medium
                                implicitHeight: Kirigami.Units.iconSizes.medium
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: Kirigami.Units.smallSpacing

                                QQC2.Label {
                                    Layout.fillWidth: true
                                    text: model.connection.localUser.displayName
                                    textFormat: Text.PlainText
                                    elide: Text.ElideRight
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 2
                                    color: Kirigami.Theme.textColor
                                }

                                QQC2.Label {
                                    Layout.fillWidth: true
                                    text: model.connection.localUserId
                                    color: Kirigami.Theme.disabledTextColor
                                    font: Kirigami.Theme.smallFont
                                    elide: Text.ElideRight
                                }
                            }

                            MobileForm.FormArrow {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                direction: MobileForm.FormArrow.Right
                            }
                        }
                    }
                }
                MobileForm.FormDelegateSeparator { below: addAccountDelegate }

                MobileForm.FormButtonDelegate {
                    id: addAccountDelegate
                    text: i18n("Add Account")
                    icon.name: "list-add"
                    onClicked: pageStack.layers.push("qrc:/WelcomePage.qml")
                }
            }
        }
    }

    Connections {
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
