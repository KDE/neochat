// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.platform
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18n("Edit Account")
    property NeoChatConnection connection

    KirigamiComponents.AvatarButton {
        id: avatar

        property OpenFileDialog fileDialog: null

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.topMargin: Kirigami.Units.largeSpacing

        // Square button
        implicitWidth: Kirigami.Units.gridUnit * 5
        implicitHeight: implicitWidth

        padding: 0

        source: root.connection && root.connection.localUser.avatarMediaId ? root.connection.makeMediaUrl("mxc://" + root.connection.localUser.avatarMediaId) : ""
        name: root.connection.localUser.displayName

        onClicked: {
            if (fileDialog) {
                return;
            }
            fileDialog = openFileDialog.createObject(this);
            fileDialog.chosen.connect(receivedSource => {
                if (!receivedSource) {
                    return;
                }
                source = receivedSource;
            });
            fileDialog.open();
        }

        QQC2.Button {
            anchors {
                bottom: parent.bottom
                right: parent.right
            }
            visible: avatar.source.toString().length === 0
            icon.name: "cloud-upload"
            text: i18n("Upload new avatar")
            display: QQC2.AbstractButton.IconOnly

            onClicked: parent.clicked()

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            anchors {
                bottom: parent.bottom
                right: parent.right
            }
            visible: avatar.source.toString().length !== 0
            icon.name: "edit-clear"
            text: i18n("Remove current avatar")
            display: QQC2.AbstractButton.IconOnly

            onClicked: avatar.source = ""

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        Component {
            id: openFileDialog

            OpenFileDialog {
                currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
                parentWindow: root.Window.window

                onAccepted: destroy()
                onRejected: destroy()
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("User information")
    }
    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: name
            label: i18n("Name:")
            text: root.connection ? root.connection.localUser.displayName : ""
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: accountLabel
            label: i18n("Label:")
            text: root.connection ? root.connection.label : ""
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "QR code for account")
            onClicked: {
                let qrMax = Qt.createComponent('org.kde.neochat', 'QrCodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                    text: "https://matrix.to/#/" + root.connection.localUser.id,
                    title: root.connection.localUser.displayName,
                    subtitle: root.connection.localUser.id,
                    avatarSource: root.connection.makeMediaUrl(root.connection.localUser.avatarUrl)
                });
                if (typeof root.closeDialog === "function") {
                    root.closeDialog();
                }
                qrMax.open();
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18n("Save")
            onClicked: {
                if (!root.connection.setAvatar(avatar.source)) {
                    showPassiveNotification("The Avatar could not be set");
                }
                if (root.connection.localUser.displayName !== name.text) {
                    root.connection.localUser.rename(name.text);
                }
                if (root.connection.label !== accountLabel.text) {
                    root.connection.label = accountLabel.text;
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Password")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            visible: root.connection !== undefined && root.connection.canChangePassword === false
            text: i18n("Your server doesn't support changing your password")
        }
        FormCard.FormDelegateSeparator {
            visible: root.connection !== undefined && root.connection.canChangePassword === false
        }
        FormCard.FormTextFieldDelegate {
            id: currentPassword
            label: i18n("Current Password:")
            enabled: root.connection !== undefined && root.connection.canChangePassword !== false
            echoMode: TextInput.Password
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: newPassword
            label: i18n("New Password:")
            enabled: root.connection !== undefined && root.connection.canChangePassword !== false
            echoMode: TextInput.Password
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: confirmPassword
            label: i18n("Confirm new Password:")
            enabled: root.connection !== undefined && root.connection.canChangePassword !== false
            echoMode: TextInput.Password
            onTextChanged: if (newPassword.text !== confirmPassword.text && confirmPassword.text.length > 0) {
                confirmPassword.status = FormCard.AbstractFormDelegate.Status.Error;
                confirmPassword.statusMessage = i18n("Passwords don't match");
            } else {
                confirmPassword.status = FormCard.AbstractFormDelegate.Status.Default;
                confirmPassword.statusMessage = '';
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18n("Save")
            enabled: currentPassword.text.length > 0 && newPassword.text.length > 0 && confirmPassword.text.length > 0
            onClicked: {
                if (newPassword.text === confirmPassword.text) {
                    root.connection.changePassword(currentPassword.text, newPassword.text);
                } else {
                    showPassiveNotification(i18n("Passwords do not match"));
                }
            }
        }
    }
    ThreePIdCard {
        connection: root.connection
        title: i18n("Email Addresses")
        medium: "email"
    }
    ThreePIdCard {
        visible: Config.phone3PId
        connection: root.connection
        title: i18n("Phone Numbers")
        medium: "msisdn"
    }
    FormCard.FormHeader {
        Layout.fillWidth: true
        title: i18n("Identity Server")
    }
    FormCard.FormCard {
        IdentityServerDelegate {
            connection: root.connection
        }
    }
    FormCard.FormHeader {
        Layout.fillWidth: true
        title: i18n("Server Information")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Homeserver url")
            description: root.connection.homeserver
        }

        /* TODO but needs first some api in Quotient
        FormCard.FormTextDelegate {
            text: i18n("Server file upload limit")
            description: root.connection.homeserver
        }

        FormCard.FormTextDelegate {
            text: i18n("Server name")
            description: root.connection.homeserver
        }

        FormCard.FormTextDelegate {
            text: i18n("Server version")
            description: root.connection.homeserver
        }*/
    }
    FormCard.FormHeader {
        title: i18nc("@title", "Account Management")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            id: deactivateAccountButton
            text: i18n("Deactivate Account")
            onClicked: {
                const component = Qt.createComponent('org.kde.neochat', 'ConfirmDeactivateAccountDialog');
                const dialog = component.createObject(QQC2.ApplicationWindow.window, {
                    connection: root.connection,
                });
                dialog.open();
            }
        }
    }

    data: Connections {
        target: root.connection
        function onPasswordStatus(status) {
            if (status === NeoChatConnection.Success) {
                showPassiveNotification(i18n("Password changed successfully"));
            } else if (status === NeoChatConnection.Wrong) {
                showPassiveNotification(i18n("Wrong password entered"));
            } else {
                showPassiveNotification(i18n("Unknown problem while trying to change password"));
            }
        }
    }
}
