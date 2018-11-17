import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral 0.1

Popup {
    property var emojiModel
    property var textArea
    property string emojiCategory: "people"

    ColumnLayout {
        anchors.fill: parent

        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            cellWidth: 36
            cellHeight: 36

            boundsBehavior: Flickable.DragOverBounds

            clip: true

            model: emojiModel.model[emojiCategory]

            delegate: ItemDelegate {
                width: 36
                height: 36

                contentItem: Text {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    font.pointSize: 20
                    font.family: "Emoji"
                    text: modelData.unicode
                }

                onClicked: textArea.insert(textArea.cursorPosition, modelData.unicode)
            }

            ScrollBar.vertical: ScrollBar {}
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2

            color: Material.accent
        }

        Row {
            Repeater {
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
                    width: 36
                    height: 36

                    contentItem: Text {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font.pointSize: 20
                        font.family: "Emoji"
                        text: label
                    }

                    onClicked: emojiCategory = category
                }
            }
        }
    }
}
