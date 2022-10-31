// SPDX-FileCopyrightText: Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Devices")

    ListView {
        model: DevicesModel {
            id: devices
        }

        anchors.fill: parent

        Kirigami.LoadingPlaceholder {
            visible: parent.count === 0 // We can assume 0 means loading since there is at least one device
            anchors.centerIn: parent
        }

        delegate: Kirigami.BasicListItem {
            text: model.displayName
            subtitle: model.id
            icon: "network-connect"
            trailing: RowLayout {
                QQC2.ToolButton {
                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Edit device name")
                        iconName: "document-edit"
                        onTriggered: {
                            renameSheet.index = model.index
                            renameSheet.name = model.displayName
                            renameSheet.open()
                        }
                    }
                }
                QQC2.ToolButton {
                    display: QQC2.AbstractButton.IconOnly
                    visible: Controller.encryptionSupported
                    action: Kirigami.Action {
                        text: i18n("Verify device")
                        iconName: "security-low-symbolic"
                        onTriggered: {
                            devices.connection.startKeyVerificationSession(model.id)
                        }
                    }
                }
                QQC2.ToolButton {
                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Logout device")
                        iconName: "edit-delete-remove"
                        onTriggered: {
                            passwordSheet.index = index
                            passwordSheet.open()
                        }
                    }
                }
            }
        }
    }

    Kirigami.OverlaySheet {
        id: passwordSheet

        property var index

        title: i18n("Remove device")
        Kirigami.FormLayout {
            QQC2.TextField {
                id: passwordField
                Kirigami.FormData.label: i18n("Password:")
                echoMode: TextInput.Password
            }
            QQC2.Button {
                text: i18n("Confirm")
                onClicked: {
                    devices.logout(passwordSheet.index, passwordField.text)
                    passwordField.text = ""
                    passwordSheet.close()
                }
            }
        }
    }

    Kirigami.OverlaySheet {
        id: renameSheet
        property int index
        property string name

        title: i18n("Edit device")
        Kirigami.FormLayout {
            QQC2.TextField {
                id: nameField
                Kirigami.FormData.label: i18n("Name:")
                text: renameSheet.name
            }
            QQC2.Button {
                text: i18n("Save")
                onClicked: {
                    devices.setName(renameSheet.index, nameField.text)
                    renameSheet.close()
                }
            }
        }
    }
}
