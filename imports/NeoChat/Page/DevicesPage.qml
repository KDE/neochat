/**
 * SPDX-FileCopyrightText: Tobias Fella <fella@posteo.de>
 *
 * SPDX-LicenseIdentifier: GPL-2.0-or-later
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Devices")

    ListView {
        model: DevicesModel {
            id: devices
        }
        delegate: Kirigami.SwipeListItem {
            leftPadding: 0
            rightPadding: 0
            Kirigami.BasicListItem {
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                text: model.displayName
                subtitle: model.id
                icon: "network-connect"
            }
            actions: [
                Kirigami.Action {
                    text: i18n("Edit device name")
                    iconName: "document-edit"
                    onTriggered: {
                        renameSheet.index = model.index
                        renameSheet.name = model.displayName
                        renameSheet.open()
                    }
                },
                Kirigami.Action {
                    text: i18n("Logout device")
                    iconName: "edit-delete-remove"
                    onTriggered: {
                        passwordSheet.index = index
                        passwordSheet.open()
                    }
                }
            ]
        }
    }

    Kirigami.OverlaySheet {
        id: passwordSheet

        property var index

        header: Kirigami.Heading {
            text: i18n("Remove device")
        }
        Kirigami.FormLayout {
            Controls.TextField {
                id: passwordField
                Kirigami.FormData.label: i18n("Password:")
                echoMode: TextInput.Password
            }
            Controls.Button {
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

        header: Kirigami.Heading {
            text: i18n("Edit device")
        }
        Kirigami.FormLayout {
            Controls.TextField {
                id: nameField
                Kirigami.FormData.label: i18n("Name:")
                text: renameSheet.name
            }
            Controls.Button {
                text: i18n("Save")
                onClicked: {
                    devices.setName(renameSheet.index, nameField.text)
                    renameSheet.close()
                }
            }
        }
    }
}
