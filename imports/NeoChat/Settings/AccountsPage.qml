// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Dialog 1.0

Kirigami.ScrollablePage {
    title: i18n("Accounts")

    actions.main: Kirigami.Action {
        text: i18n("Add an account")
        icon.name: "list-add-user"
        onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/WelcomePage.qml")
        visible: !pageSettingStack.wideMode
    }

    ListView {
        model: AccountRegistry
        delegate: Kirigami.SwipeListItem {
            leftPadding: 0
            rightPadding: 0
            Kirigami.BasicListItem {
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                text: model.connection.localUser.displayName
                labelItem.textFormat: Text.PlainText
                subtitle: model.connection.localUserId
                icon: model.connection.localUser.avatarMediaId ? ("image://mxc/" + model.connection.localUser.avatarMediaId) : "im-user"

                onClicked: {
                    Controller.activeConnection = model.connection
                    pageStack.layers.pop()
                }
            }
            actions: [
                Kirigami.Action {
                    text: i18n("Edit this account")
                    iconName: "document-edit"
                    onTriggered: {
                        userEditSheet.connection = model.connection
                        userEditSheet.open()
                    }
                },
                Kirigami.Action {
                    text: i18n("Logout")
                    iconName: "im-kick-user"
                    onTriggered: {
                        Controller.logout(model.connection, true)
                        if(Controller.accountCount === 1)
                            pageStack.layers.pop()
                    }
                }
            ]
        }
    }

    footer: Controls.ToolBar {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.ActionToolBar {
            alignment: Qt.AlignRight
            rightPadding: Kirigami.Units.smallSpacing
            width: parent.width
            flat: false
            actions: Kirigami.Action {
                text: i18n("Add an account")
                icon.name: "list-add-user"
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/WelcomePage.qml")
            }
        }
    }
    Connections {
        target: Controller
        function onConnectionAdded() {
            if (pageStack.layers.depth > 2)
                pageStack.layers.pop()
        }
        function onPasswordStatus(status) {
            if(status == Controller.Success)
                showPassiveNotification(i18n("Password changed successfully"))
            else if(status == Controller.Wrong)
                showPassiveNotification(i18n("Wrong password entered"))
            else
                showPassiveNotification(i18n("Unknown problem while trying to change password"))
        }
    }

    Component {
        id: openFileDialog

        OpenFileDialog {
            folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        }
    }

    Kirigami.OverlaySheet {
        id: userEditSheet

        property var connection

        title: i18n("Edit Account")

        Kirigami.FormLayout {
            RowLayout {
                Kirigami.Avatar {
                    id: avatar
                    source: userEditSheet.connection && userEditSheet.connection.localUser.avatarMediaId ? ("image://mxc/" + userEditSheet.connection.localUser.avatarMediaId) : ""

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
                    visible: avatar.source.length !== 0
                    icon.name: "edit-clear"

                    onClicked: avatar.source = ""
                }
                Kirigami.FormData.label: i18n("Avatar:")
            }
            Controls.TextField {
                id: name
                text: userEditSheet.connection ? userEditSheet.connection.localUser.displayName : ""
                Kirigami.FormData.label: i18n("Name:")
            }
            Controls.TextField {
                id: currentPassword
                Kirigami.FormData.label: i18n("Current Password:")
                enabled: userEditSheet.connection !== undefined && userEditSheet.connection.canChangePassword !== false
                echoMode: TextInput.Password
            }
            Controls.TextField {
                id: newPassword
                Kirigami.FormData.label: i18n("New Password:")
                enabled: userEditSheet.connection !== undefined && userEditSheet.connection.canChangePassword !== false
                echoMode: TextInput.Password

            }
            Controls.TextField {
                id: confirmPassword
                Kirigami.FormData.label: i18n("Confirm new Password:")
                enabled: userEditSheet.connection !== undefined && userEditSheet.connection.canChangePassword !== false
                echoMode: TextInput.Password
            }

            RowLayout {
                Controls.Button {
                    text: i18n("Save")
                    onClicked: {
                        if(!Controller.setAvatar(userEditSheet.connection, avatar.source))
                            showPassiveNotification("The Avatar could not be set")
                        if(userEditSheet.connection.localUser.displayName !== name.text)
                            userEditSheet.connection.localUser.rename(name.text)
                        if(currentPassword.text !== "" && newPassword.text !== "" && confirmPassword.text !== "") {
                            if(newPassword.text === confirmPassword.text) {
                                Controller.changePassword(userEditSheet.connection, currentPassword.text, newPassword.text)
                            } else {
                                showPassiveNotification(i18n("Passwords do not match"))
                                return
                            }
                        }
                        userEditSheet.close()
                        currentPassword.text = ""
                        newPassword.text = ""
                        confirmPassword.text = ""
                    }
                }
                Controls.Button {
                    text: i18n("Cancel")
                    onClicked: {
                        userEditSheet.close()
                        avatar.source = userEditSheet.connection.localUser.avatarMediaId ? ("image://mxc/" + userEditSheet.connection.localUser.avatarMediaId) : ""
                        currentPassword.text = ""
                        newPassword.text = ""
                        confirmPassword.text = ""
                    }
                }
            }
        }
    }
}
