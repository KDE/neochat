// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Dialog 1.0

Kirigami.ScrollablePage {
    id: root
    title: i18n("Edit Account")
    property var connection

    ColumnLayout {
        Kirigami.FormLayout {
            RowLayout {
                Kirigami.Avatar {
                    id: avatar
                    source: root.connection && root.connection.localUser.avatarMediaId ? ("image://mxc/" + root.connection.localUser.avatarMediaId) : ""

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        property var fileDialog: null;
                        onClicked: {
                            if (fileDialog != null) {
                                return;
                            }

                            fileDialog = openFileDialog.createObject(Controls.ApplicationWindow.Overlay)

                            fileDialog.chosen.connect(function(receivedSource) {
                                mouseArea.fileDialog = null;
                                if (!receivedSource) {
                                    return;
                                }
                                parent.source = receivedSource;
                            });
                            fileDialog.onRejected.connect(function() {
                                mouseArea.fileDialog = null;
                            });
                            fileDialog.open();
                        }
                    }
                }
                Controls.Button {
                    visible: avatar.source.toString().length !== 0
                    icon.name: "edit-clear"

                    onClicked: avatar.source = ""
                }
                Kirigami.FormData.label: i18n("Avatar:")
            }
            Controls.TextField {
                id: name
                text: root.connection ? root.connection.localUser.displayName : ""
                Kirigami.FormData.label: i18n("Name:")
            }
            Controls.TextField {
                id: accountLabel
                text: root.connection ? root.connection.localUser.accountLabel : ""
                Kirigami.FormData.label: i18n("Label:")
            }
            Controls.TextField {
                id: currentPassword
                Kirigami.FormData.label: i18n("Current Password:")
                enabled: roto.connection !== undefined && root.connection.canChangePassword !== false
                echoMode: TextInput.Password
            }
            Controls.TextField {
                id: newPassword
                Kirigami.FormData.label: i18n("New Password:")
                enabled: root.connection !== undefined && root.connection.canChangePassword !== false
                echoMode: TextInput.Password

            }
            Controls.TextField {
                id: confirmPassword
                Kirigami.FormData.label: i18n("Confirm new Password:")
                enabled: root.connection !== undefined && root.connection.canChangePassword !== false
                echoMode: TextInput.Password
            }
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }

        Controls.Button {
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
                if (root.connection.localUser.accountLabel !== accountLabel.text) {
                    root.connection.localUser.setAccountLabel(accountLabel.text);
                }
                if(currentPassword.text !== "" && newPassword.text !== "" && confirmPassword.text !== "") {
                    if(newPassword.text === confirmPassword.text) {
                        Controller.changePassword(root.connection, currentPassword.text, newPassword.text);
                    } else {
                        showPassiveNotification(i18n("Passwords do not match"));
                        return;
                    }
                }
                root.closeDialog();
            }
        }
        Controls.Button {
            text: i18n("Cancel")
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            onClicked: root.closeDialog();
        }
    }
}
