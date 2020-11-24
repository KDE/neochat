/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0

import org.kde.neochat 1.0

ColumnLayout {
    property string emojiCategory: "history"
    property var textArea
    property var emojiModel

    signal choosen(string emoji)

    spacing: 0

    ListView {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2 + 2 // for the focus line

        boundsBehavior: Flickable.DragOverBounds

        clip: true

        orientation: ListView.Horizontal

        model: ListModel {
            ListElement { label: "⌛️"; category: "history" }
            ListElement { label: "😏"; category: "people" }
            ListElement { label: "🌲"; category: "nature" }
            ListElement { label: "🍛"; category: "food"}
            ListElement { label: "🚁"; category: "activity" }
            ListElement { label: "🚅"; category: "travel" }
            ListElement { label: "💡"; category: "objects" }
            ListElement { label: "🔣"; category: "symbols" }
            ListElement { label: "🏁"; category: "flags" }
        }

        delegate: ItemDelegate {
            width: Kirigami.Units.gridUnit * 2
            height: Kirigami.Units.gridUnit * 2

            contentItem: Kirigami.Heading {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                level: 1

                font.family: 'emoji'
                text: label
            }

            Rectangle {
                anchors.bottom: parent.bottom

                width: parent.width
                height: 2

                visible: emojiCategory === category

                color: Kirigami.Theme.focusColor
            }

            onClicked: emojiCategory = category
        }
    }

    Kirigami.Separator {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }

    GridView {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 8
        Layout.fillHeight: true

        cellWidth: Kirigami.Units.gridUnit * 2
        cellHeight: Kirigami.Units.gridUnit * 2

        boundsBehavior: Flickable.DragOverBounds

        clip: true

        model: {
            switch (emojiCategory) {
            case "history":
                return emojiModel.history
            case "people":
                return emojiModel.people
            case "nature":
                return emojiModel.nature
            case "food":
                return emojiModel.food
            case "activity":
                return emojiModel.activity
            case "travel":
                return emojiModel.travel
            case "objects":
                return emojiModel.objects
            case "symbols":
                return emojiModel.symbols
            case "flags":
                return emojiModel.flags
            }
            return null
        }

        delegate: ItemDelegate {
            width: Kirigami.Units.gridUnit * 2
            height: Kirigami.Units.gridUnit * 2

            contentItem: Kirigami.Heading {
                level: 1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.family: 'emoji'
                text: modelData.unicode
            }

            onClicked: {
                choosen(modelData.unicode)
                emojiModel.emojiUsed(modelData)
            }
        }

        ScrollBar.vertical: ScrollBar {}
    }
}
