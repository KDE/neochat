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
    readonly property bool hasUnsavedChanges: root.connection.localUser.displayName !== name.text
        || avatar.source != avatar.findOriginalAvatarUrl()
        || root.connection.label !== accountLabel.text;

    function resetChanges(): void {
        name.text = root.connection ? root.connection.localUser.displayName : "";
        accountLabel.text = root.connection ? root.connection.label : "";
        avatar.source = avatar.findOriginalAvatarUrl();
    }

    function saveChanges(): void {
        if (avatar.source != avatar.findOriginalAvatarUrl() && !root.connection.setAvatar(avatar.source)) {
            (root.Window.window as Kirigami.ApplicationWindow).showPassiveNotification(i18nc("@info", "New avatar could not be set."));
        }
        if (root.connection.localUser.displayName !== name.text) {
            root.connection.localUser.rename(name.text);
        }
        if (root.connection.label !== accountLabel.text) {
            root.connection.label = accountLabel.text;
        }
    }

    function checkForUnsavedChanges(): bool {
        if (root.hasUnsavedChanges) {
            resetChangesDialog.open();
            return true;
        }
        return false;
    }

    onBackRequested: event => {
        if (checkForUnsavedChanges(event)) {
            event.accepted = true; // Prevent the page from popping
        }
    }

    Connections {
        target: root.Window.window

        function onClosing(event): void {
            if (root.checkForUnsavedChanges(event)) {
                event.accepted = false; // Prevent the window from closing
            }
        }
    }

    Kirigami.PromptDialog {
        id: resetChangesDialog

        parent: root.QQC2.Overlay.overlay
        preferredWidth: Kirigami.Units.gridUnit * 24

        title: i18nc("@title:dialog Apply unsaved settings", "Apply Settings")
        subtitle: i18nc("@info", "There are unsaved changes to user information. Apply the changes or discard them?")

        standardButtons: QQC2.Dialog.Cancel

        footer: QQC2.DialogButtonBox {
            QQC2.Button {
                text: i18nc("@action:button As in 'Remove this device'", "Apply")
                icon.name: "dialog-ok-apply"

                onClicked: {
                    root.saveChanges();
                    resetChangesDialog.close();
                }

                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.ApplyRole
            }

            QQC2.Button {
                text: i18nc("@action:button As in 'Remove this device'", "Reset")
                icon.name: "edit-reset-symbolic"

                onClicked: {
                    root.resetChanges();
                    resetChangesDialog.close();
                }

                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.ResetRole
            }
        }
    }

    Component.onCompleted: resetChanges()

    KirigamiComponents.AvatarButton {
        id: avatar

        property OpenFileDialog fileDialog: null

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.topMargin: Kirigami.Units.largeSpacing

        // Square button
        implicitWidth: Kirigami.Units.gridUnit * 5
        implicitHeight: implicitWidth

        padding: 0

        name: root.connection.localUser.displayName

        function findOriginalAvatarUrl(): string {
            return root.connection && (root.connection.localUser.avatarUrl.toString().length > 0 ? root.connection.makeMediaUrl(root.connection.localUser.avatarUrl) : "");
        }

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
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: accountLabel
            label: i18n("Label:")
            placeholderText: i18n("Work")
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextDelegate {
            id: userIdDelegate
            text: i18nc("@info:label", "User ID")
            description: root.connection.localUserId

            contentItem.children: QQC2.Button {
                text: i18nc("@action:button", "Copy user ID to clipboard")
                icon.name: "edit-copy"
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    Clipboard.saveText(root.connection.localUserId);
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Reset Changes")
            icon.name: "edit-reset-symbolic"
            enabled: root.hasUnsavedChanges
            onClicked: root.resetChanges()
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button Save changes to user", "Save")
            icon.name: "document-save-symbolic"
            enabled: root.hasUnsavedChanges
            onClicked: root.saveChanges()
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
