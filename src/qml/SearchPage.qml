// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.ScrollablePage {
    id: root

    property NeoChatRoom currentRoom
    required property NeoChatConnection connection

    title: i18nc("@action:title", "Search Messages")

    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    SearchModel {
        id: searchModel
        connection: root.connection
        searchText: searchField.text
        room: root.currentRoom
    }

    header: QQC2.Control {
        padding: Kirigami.Units.largeSpacing

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor

            Kirigami.Separator {
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                    right: parent.right
                }
            }
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Kirigami.SearchField {
                id: searchField
                focus: true
                Layout.fillWidth: true
                Keys.onEnterPressed: searchButton.clicked()
                Keys.onReturnPressed: searchButton.clicked()
            }
            QQC2.Button {
                id: searchButton
                onClicked: searchModel.search()
                icon.name: "search"
            }
        }
    }

    ListView {
        id: messageListView
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0
        verticalLayoutDirection: ListView.BottomToTop

        section.property: "section"

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: searchField.text.length === 0 && messageListView.count === 0
            text: i18n("Enter a text to start searching")
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: searchField.text.length > 0 && messageListView.count === 0 && !searchModel.searching
            text: i18n("No results found")
        }

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: searchModel.searching
        }

        model: searchModel
        delegate: EventDelegate {
            connection: root.connection
        }
    }
}
