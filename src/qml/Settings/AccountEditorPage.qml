// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import QtQuick.Window 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents
import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root
    title: i18n("Edit Account")
    property var connection

    readonly property bool compact: width > Kirigami.Units.gridUnit * 30 ? 2 : 1

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        spacing: 0

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
        }

        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("User information")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormTextFieldDelegate {
                    id: name
                    label: i18n("Name:")
                    text: root.connection ? root.connection.localUser.displayName : ""
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: accountLabel
                    label: i18n("Label:")
                    text: root.connection ? Controller.activeAccountLabel : ""
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.AbstractFormDelegate {
                    Layout.fillWidth: true
                    background: null
                    padding: Kirigami.Units.smallSpacing
                    contentItem: RowLayout {
                        Item {
                            Layout.fillWidth: true
                        }
                        QQC2.Button {
                            text: i18n("Save")
                            Layout.bottomMargin: Kirigami.Units.smallSpacing
                            Layout.topMargin: Kirigami.Units.smallSpacing
                            onClicked: {
                                if (!Controller.setAvatar(root.connection, avatar.source)) {
                                    showPassiveNotification("The Avatar could not be set");
                                }
                                if (root.connection.localUser.displayName !== name.text) {
                                    root.connection.localUser.rename(name.text);
                                }
                                if (Controller.activeAccountLabel !== accountLabel.text) {
                                    Controller.activeAccountLabel = accountLabel.text;
                                }
                            }
                        }
                    }
                }
            }
        }

        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("Password")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormTextDelegate {
                    visible: root.connection !== undefined && root.connection.canChangePassword === false
                    text: i18n("Your server doesn't support changing your password")
                }
                MobileForm.FormDelegateSeparator { visible: root.connection !== undefined && root.connection.canChangePassword === false }
                MobileForm.FormTextFieldDelegate {
                    id: currentPassword
                    label: i18n("Current Password:")
                    enabled: root.connection !== undefined && root.connection.canChangePassword !== false
                    echoMode: TextInput.Password
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: newPassword
                    label: i18n("New Password:")
                    enabled: root.connection !== undefined && root.connection.canChangePassword !== false
                    echoMode: TextInput.Password
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: confirmPassword
                    label: i18n("Confirm new Password:")
                    enabled: root.connection !== undefined && root.connection.canChangePassword !== false
                    echoMode: TextInput.Password
                    onTextChanged: if (newPassword.text !== confirmPassword.text && confirmPassword.text.length > 0) {
                        confirmPassword.status = MobileForm.AbstractFormDelegate.Status.Error;
                        confirmPassword.statusMessage = i18n("Passwords don't match");
                    } else {
                        confirmPassword.status = MobileForm.AbstractFormDelegate.Status.Default;
                        confirmPassword.statusMessage = '';
                    }
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.AbstractFormDelegate {
                    Layout.fillWidth: true
                    background: null
                    contentItem: RowLayout {
                        Item {
                            Layout.fillWidth: true
                        }
                        QQC2.Button {
                            text: i18n("Save")
                            Layout.bottomMargin: Kirigami.Units.smallSpacing
                            Layout.topMargin: Kirigami.Units.smallSpacing
                            enabled: currentPassword.text.length > 0 && newPassword.text.length > 0 && confirmPassword.text.length > 0
                            onClicked: {
                                if (newPassword.text === confirmPassword.text) {
                                    Controller.changePassword(root.connection, currentPassword.text, newPassword.text);
                                } else {
                                    showPassiveNotification(i18n("Passwords do not match"));
                                }
                            }
                        }
                        QQC2.Button {
                            text: i18n("Cancel")
                            Layout.rightMargin: Kirigami.Units.smallSpacing
                            Layout.bottomMargin: Kirigami.Units.smallSpacing
                            Layout.topMargin: Kirigami.Units.smallSpacing
                            onClicked: root.closeDialog();
                        }
                    }
                }
            }
        }

        MobileForm.FormHeader {
            Layout.fillWidth: true
            title: i18n("Server Information")
        }
        MobileForm.FormCard {
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormTextDelegate {
                    text: i18n("Homeserver url")
                    description: root.connection.homeserver
                }

                /** TODO but needs first some api in Quotient
                MobileForm.FormTextDelegate {
                    text: i18n("Server file upload limit")
                    description: root.connection.homeserver
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Server name")
                    description: root.connection.homeserver
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Server version")
                    description: root.connection.homeserver
                }*/
            }
        }
    }
    Component {
        id: openFileDialog

        OpenFileDialog {
            folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
            parentWindow: root.Window.window
        }
    }
}
