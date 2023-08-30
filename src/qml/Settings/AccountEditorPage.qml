// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import QtQuick.Window 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents
import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    title: i18n("Edit Account")
    property NeoChatConnection connection

    QQC2.RoundButton {
        property var fileDialog: null;

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.topMargin: Kirigami.Units.largeSpacing

        // Square button
        implicitWidth: Kirigami.Units.gridUnit * 5
        implicitHeight: implicitWidth

        padding: 0

        contentItem: KirigamiComponents.Avatar {
            id: avatar
            source: root.connection && root.connection.localUser.avatarMediaId ? ("image://mxc/" + root.connection.localUser.avatarMediaId) : ""
            name: root.connection.localUser.displayName
        }

        onClicked: {
            if (fileDialog != null) {
                return;
            }

            fileDialog = openFileDialog.createObject(QQC2.ApplicationWindow.Overlay)
            fileDialog.chosen.connect(function(receivedSource) {
                if (!receivedSource) {
                    return;
                }
                avatar.source = receivedSource;
            });
            fileDialog.onRejected.connect(function() {
                mouseArea.fileDialog = null;
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

            onClicked: parent.onClicked()

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
                folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
                parentWindow: root.Window.window
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
        FormCard.FormDelegateSeparator { visible: root.connection !== undefined && root.connection.canChangePassword === false }
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
            onClicked: pageStack.pushDialogLayer("qrc:/ConfirmDeactivateAccountDialog.qml", {connection: root.connection}, {title: i18nc("@title", "Confirm Deactivating Account")})
        }
    }
}
