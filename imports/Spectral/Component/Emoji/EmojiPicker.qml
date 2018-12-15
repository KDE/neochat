import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.4

import Spectral.Component 2.0

import Spectral 0.1
import Spectral.Setting 0.1

ColumnLayout {
    property string emojiCategory: "people"
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

        model: emojiModel.model[emojiCategory]

        delegate: ItemDelegate {
            width: 48
            height: 48

            contentItem: Label {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pixelSize: 32
                text: modelData.unicode
            }

            onClicked: textArea.insert(textArea.cursorPosition, modelData.unicode)
        }

        ScrollBar.vertical: ScrollBar {}
    }
}
