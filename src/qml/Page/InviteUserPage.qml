// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property var room

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

        Button {
            visible: identifierField.isUserID

            text: i18n("Add")
            highlighted: true

            onClicked: {
                room.inviteToRoom(identifierField.text)
            }
        }
    }

    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true

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

        delegate: Kirigami.AbstractListItem {
            id: delegate
            property bool inRoom: room && room.containsUser(userID)

            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing

            contentItem: RowLayout {
                Kirigami.Avatar {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    source: model.avatar ? ("image://mxc/" + model.avatar) : ""
                    name: model.name
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    spacing: 0

                    Kirigami.Heading {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        level: 3

                        text: name
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        text: userID
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }
                }

                ToolButton {
                    visible: !inRoom
                    icon.name: "document-send"
                    text: i18n("Send invitation")

                    onClicked: {
                        room.inviteToRoom(userID);
                        applicationWindow().pageStack.layers.pop();
                    }
                }
            }
        }
    }
}
