// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: searchPage

    property var currentRoom

    title: i18nc("@action:title", "Search Messages")

    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    SearchModel {
        id: searchModel
        connection: Controller.activeConnection
        searchText: searchField.text
        room: searchPage.currentRoom
    }

    header: RowLayout {
        Kirigami.SearchField {
            id: searchField
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            Keys.onEnterPressed: searchButton.clicked()
            Keys.onReturnPressed: searchButton.clicked()
        }
        QQC2.Button {
            id: searchButton
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            onClicked: searchModel.search()
            icon.name: "search"
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
        delegate: EventDelegate {}
    }
}
