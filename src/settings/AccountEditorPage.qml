// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Edit Account")
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

        // Note: User::avatarUrl does not set user_id, and thus cannot be used directly here. Hence the makeMediaUrl.
        source: root.connection && (root.connection.localUser.avatarUrl.toString().length > 0 ? root.connection.makeMediaUrl(root.connection.localUser.avatarUrl) : "")
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
                currentFolder: Core.StandardPaths.standardLocations(Core.StandardPaths.PicturesLocation)[0]
                parentWindow: root.Window.window

                onAccepted: destroy()
                onRejected: destroy()
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "User Information")
    }
    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: name
            label: i18n("Display Name:")
            text: root.connection ? root.connection.localUser.displayName : ""
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: accountLabel
            label: i18n("Label:")
            placeholderText: i18n("Work")
            text: root.connection ? root.connection.label : ""
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Show QR Code")
            icon.name: "view-barcode-qr-symbolic"
            onClicked: {
                let qrMax = Qt.createComponent('org.kde.neochat', 'QrCodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                    text: "https://matrix.to/#/" + root.connection.localUser.id,
                    title: root.connection.localUser.displayName,
                    subtitle: root.connection.localUser.id,
                    // Note: User::avatarUrl does not set user_id, and thus cannot be used directly here. Hence the makeMediaUrl.
                    avatarSource: root.connection && (root.connection.localUser.avatarUrl.toString().length > 0 ? root.connection.makeMediaUrl(root.connection.localUser.avatarUrl) : "")
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
            icon.name: "document-save-symbolic"
            onClicked: {
                if (!root.connection.setAvatar(avatar.source)) {
                    (root.Window.window as Kirigami.ApplicationWindow).showPassiveNotification("The Avatar could not be set");
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
        title: i18nc("@title:group", "Password")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            visible: root.connection !== undefined && root.connection.canChangePassword === false
            text: i18nc("@info", "Your server doesn't support changing your password")
        }
        FormCard.FormDelegateSeparator {
            visible: root.connection !== undefined && root.connection.canChangePassword === false
        }
        FormCard.FormTextFieldDelegate {
            id: currentPassword
            label: i18nc("@label:textbox", "Current Password:")
            enabled: root.connection !== undefined && root.connection.canChangePassword !== false
            echoMode: TextInput.Password
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: newPassword
            label: i18nc("@label:textbox", "New Password:")
            enabled: root.connection !== undefined && root.connection.canChangePassword !== false
            echoMode: TextInput.Password
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: confirmPassword
            label: i18nc("@label:textbox", "Confirm new Password:")
            enabled: root.connection !== undefined && root.connection.canChangePassword !== false
            echoMode: TextInput.Password
            onTextChanged: if (newPassword.text !== confirmPassword.text && confirmPassword.text.length > 0) {
                confirmPassword.status = Kirigami.MessageType.Error;
                confirmPassword.statusMessage = i18nc("@info", "Passwords don't match");
            } else {
                confirmPassword.status = Kirigami.MessageType.Information;
                confirmPassword.statusMessage = '';
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18n("Save")
            icon.name: "document-save-symbolic"
            enabled: currentPassword.text.length > 0 && newPassword.text.length > 0 && confirmPassword.text.length > 0 && newPassword.text === confirmPassword.text
            onClicked: {
                if (newPassword.text === confirmPassword.text) {
                    root.connection.changePassword(currentPassword.text, newPassword.text);
                }
            }
        }
    }
    ThreePIdCard {
        connection: root.connection
        title: i18nc("@title:group", "Email Addresses")
        medium: "email"
    }
    ThreePIdCard {
        visible: NeoChatConfig.phone3PId
        connection: root.connection
        title: i18nc("@title:group", "Phone Numbers")
        medium: "msisdn"
    }
    FormCard.FormHeader {
        Layout.fillWidth: true
        title: i18nc("@title:group", "Identity Server")
    }
    FormCard.FormCard {
        IdentityServerDelegate {
            connection: root.connection
        }
    }
    FormCard.FormHeader {
        Layout.fillWidth: true
        title: i18nc("@title:group", "Server Information")
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
        title: i18nc("@title:group", "Account Management")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            id: deactivateAccountButton
            text: i18nc("@action:button", "Deactivate Account…")
            icon.name: "trash-empty-symbolic"
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
                confirmPassword.status = Kirigami.MessageType.Positive
                confirmPassword.statusMessage = i18nc("@info", "Password changed successfully");
            } else if (status === NeoChatConnection.Wrong) {
                confirmPassword.status = Kirigami.MessageType.Error
                confirmPassword.statusMessage = i18nc("@info", "Invalid password");
            } else {
                confirmPassword.status = Kirigami.MessageType.Error
                confirmPassword.statusMessage = i18nc("@info", "Unknown problem while trying to change password");
            }
        }
    }
}
