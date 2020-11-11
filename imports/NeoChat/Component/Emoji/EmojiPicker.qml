/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import NeoChat.Component 1.0

import org.kde.neochat 1.0
import NeoChat.Setting 1.0

ColumnLayout {
    property string emojiCategory: "history"
    property var textArea
    property var emojiModel

    spacing: 0

    ListView {
        Layout.fillWidth: true
        Layout.preferredHeight: 48
        Layout.leftMargin: 24
        Layout.rightMargin: 24

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
            width: 64
            height: 48

            contentItem: Label {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pixelSize: 24
                font.family: 'emoji'
                text: label
            }

            Rectangle {
                anchors.bottom: parent.bottom

                width: parent.width
                height: 2

                visible: emojiCategory === category

                color: Material.accent
            }

            onClicked: emojiCategory = category
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        Layout.leftMargin: 12
        Layout.rightMargin: 12

        color: MSettings.darkTheme ? "#424242" : "#e7ebeb"
    }

    GridView {
        Layout.fillWidth: true
        Layout.preferredHeight: 180

        cellWidth: 48
        cellHeight: 48

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
            width: 48
            height: 48

            contentItem: Label {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pixelSize: 32
                font.family: 'emoji'
                text: modelData.unicode
            }

            onClicked: {
                textArea.insert(textArea.cursorPosition, modelData.unicode)
                emojiModel.emojiUsed(modelData)
            }
        }

        ScrollBar.vertical: ScrollBar {}
    }
}
