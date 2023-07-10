// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Devices")
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        spacing: 0
        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("Devices")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.AbstractFormDelegate {
                    Layout.fillWidth: true
                    visible: Controller.activeConnection && deviceRepeater.count === 0 // We can assume 0 means loading since there is at least one device
                    contentItem: Kirigami.LoadingPlaceholder { }
                }
                Repeater {
                    id: deviceRepeater
                    model: DevicesModel {
                        id: devices
                    }

                    Kirigami.LoadingPlaceholder {
                        visible: parent.count === 0 // We can assume 0 means loading since there is at least one device
                        anchors.centerIn: parent
                    }

                    delegate: MobileForm.AbstractFormDelegate {
                        id: deviceDelegate

                        property bool editDeviceName: false

                        Layout.fillWidth: true

                        onClicked: deviceDelegate.editDeviceName = true

                        contentItem: RowLayout {
                            spacing: Kirigami.Units.largeSpacing

                            Kirigami.Icon {
                                source: "network-connect"
                                implicitWidth: Kirigami.Units.iconSizes.medium
                                implicitHeight: Kirigami.Units.iconSizes.medium
                            }
                            ColumnLayout {
                                id: deviceLabel
                                Layout.fillWidth: true
                                spacing: Kirigami.Units.smallSpacing
                                visible: !deviceDelegate.editDeviceName

                                QQC2.Label {
                                    Layout.fillWidth: true
                                    text: model.displayName
                                    elide: Text.ElideRight
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 2
                                }

                                QQC2.Label {
                                    Layout.fillWidth: true
                                    text: model.id + ", Last activity: " +  (new Date(model.lastTimestamp)).toLocaleString(Qt.locale(), Locale.ShortFormat)
                                    color: Kirigami.Theme.disabledTextColor
                                    font: Kirigami.Theme.smallFont
                                    elide: Text.ElideRight
                                    visible: text !== ""
                                }
                            }
                            Kirigami.ActionTextField {
                                id: nameField
                                Accessible.description: i18n("New device name")
                                Layout.fillWidth: true
                                Layout.preferredHeight: deviceLabel.implicitHeight
                                visible: deviceDelegate.editDeviceName

                                text: model.displayName

                                rightActions: [
                                    Kirigami.Action {
                                        text: i18n("Cancel editing display name")
                                        icon.name: "edit-delete-remove"
                                        onTriggered: {
                                            deviceDelegate.editDeviceName = false
                                        }
                                    },
                                    Kirigami.Action {
                                        text: i18n("Confirm new display name")
                                        icon.name: "checkmark"
                                        visible: nameField.text != model.displayName
                                        onTriggered: {
                                            devices.setName(model.index, nameField.text)
                                        }
                                    }
                                ]

                                onAccepted: devices.setName(model.index, nameField.text)
                            }
                            QQC2.ToolButton {
                                display: QQC2.AbstractButton.IconOnly
                                action: Kirigami.Action {
                                    id: editDeviceAction
                                    text: i18n("Edit device name")
                                    icon.name: "document-edit"
                                    onTriggered: deviceDelegate.editDeviceName = true
                                }
                                QQC2.ToolTip {
                                    text: editDeviceAction.text
                                    delay: Kirigami.Units.toolTipDelay
                                }
                            }
                            QQC2.ToolButton {
                                display: QQC2.AbstractButton.IconOnly
                                visible: Controller.encryptionSupported
                                action: Kirigami.Action {
                                    id: verifyDeviceAction
                                    text: i18n("Verify device")
                                    icon.name: "security-low-symbolic"
                                    onTriggered: {
                                        devices.connection.startKeyVerificationSession(devices.connection.localUserId, model.id)
                                    }
                                }
                                QQC2.ToolTip {
                                    text: verifyDeviceAction.text
                                    delay: Kirigami.Units.toolTipDelay
                                }
                            }
                            QQC2.ToolButton {
                                display: QQC2.AbstractButton.IconOnly
                                action: Kirigami.Action {
                                    id: logoutDeviceAction
                                    text: i18n("Logout device")
                                    icon.name: "edit-delete-remove"
                                    onTriggered: {
                                        passwordSheet.index = index
                                        passwordSheet.open()
                                    }
                                }
                                QQC2.ToolTip {
                                    text: logoutDeviceAction.text
                                    delay: Kirigami.Units.toolTipDelay
                                }
                            }
                        }
                    }
                }
            }
        }
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.maximumWidth: Kirigami.Units.gridUnit * 30
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Please login to view the signed-in devices for your account.")
            type: Kirigami.MessageType.Information
            visible: !Controller.activeConnection
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
}
