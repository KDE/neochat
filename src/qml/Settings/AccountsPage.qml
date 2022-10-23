// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Accounts")

    actions.main: Kirigami.Action {
        text: i18n("Add an account")
        icon.name: "list-add-user"
        onTriggered: pageStack.layers.push("qrc:/WelcomePage.qml")
        visible: !pageSettingStack.wideMode
    }

    ListView {
        model: AccountRegistry
        anchors.fill: parent
        delegate: Kirigami.BasicListItem {
            text: model.connection.localUser.displayName
            labelItem.textFormat: Text.PlainText
            subtitle: model.connection.localUserId

            leading: Kirigami.Avatar {
                name: model.connection.localUser.displayName ?? model.connection.localUser.id
                source: model.connection.localUser.avatarMediaId ? ("image://mxc/" + model.connection.localUser.avatarMediaId) : ""
                width: height
            }

            onClicked: {
                Controller.activeConnection = model.connection;
                pageStack.layers.pop();
            }

            trailing: RowLayout {
                Controls.ToolButton {
                    display: Controls.AbstractButton.IconOnly
                    Controls.ToolTip {
                        text: parent.action.text
                    }
                    action: Kirigami.Action {
                        text: i18n("Edit this account")
                        iconName: "document-edit"
                        onTriggered: pageSettingStack.pushDialogLayer(Qt.resolvedUrl('./AccountEditorPage.qml'), {
                            connection: model.connection
                        }, {
                            title: i18n("Account editor")
                        });
                    }
                }
                Controls.ToolButton {
                    display: Controls.AbstractButton.IconOnly
                    Controls.ToolTip {
                        text: parent.action.text
                    }
                    action: Kirigami.Action {
                        text: i18n("Logout")
                        iconName: "im-kick-user"
                        onTriggered: {
                            Controller.logout(model.connection, true);
                            if (Controller.accountCount === 1) {
                                pageStack.layers.pop();
                            }
                        }
                    }
                }
            }
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
                onTriggered: pageStack.layers.push("qrc:/WelcomePage.qml")
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
            if (status === Controller.Success) {
                showPassiveNotification(i18n("Password changed successfully"));
            } else if (status === Controller.Wrong) {
                showPassiveNotification(i18n("Wrong password entered"));
            } else {
                showPassiveNotification(i18n("Unknown problem while trying to change password"));
            }
        }
    }
}
