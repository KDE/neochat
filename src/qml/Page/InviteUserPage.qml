// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property NeoChatRoom room

    title: i18n("Invite a User")

    actions {
        main: Kirigami.Action {
            icon.name: "dialog-close"
            text: i18nc("@action", "Cancel")
            onTriggered: applicationWindow().pageStack.layers.pop()
        }
    }
    header: RowLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.largeSpacing

        Kirigami.SearchField {
            id: identifierField
            property bool isUserID: text.match(/@(.+):(.+)/g)
            Layout.fillWidth: true

            placeholderText: i18n("Find a user...")
            onAccepted: userDictListModel.search()
        }

        QQC2.Button {
            visible: identifierField.isUserID

            text: i18n("Add")
            highlighted: true

            onClicked: {
                room.inviteToRoom(identifierField.text)
            }
        }
    }

    ListView {
        id: userDictListView

        clip: true

        model: UserDirectoryListModel {
            id: userDictListModel

            connection: root.room.connection
            keyword: identifierField.text
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent

            visible: userDictListView.count < 1

            text: i18n("No users available")
        }

        delegate: Kirigami.BasicListItem {
            id: delegate
            property bool inRoom: room && room.containsUser(userID)

            label: model.name
            subtitle: model.userID

            leading: Kirigami.Avatar {
                implicitWidth: height
                source: model.avatar ? ("image://mxc/" + model.avatar) : ""
                name: model.name
            }
            trailing: QQC2.ToolButton {
                id: inviteButton
                icon.name: "document-send"
                text: i18n("Send invitation")
                checkable: true
                checked: inRoom
                opacity: inRoom ? 0.5 : 1

                onToggled: {
                    if (inRoom) {
                        checked = true
                    } else {
                        room.inviteToRoom(model.userID);
                        applicationWindow().pageStack.layers.pop();
                    }
                }

                QQC2.ToolTip.text: !inRoom ? text : i18n("User is either already a member or has been invited")
                QQC2.ToolTip.visible: inviteButton.hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
}
