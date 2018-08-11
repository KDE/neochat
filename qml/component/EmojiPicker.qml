import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtQuick.XmlListModel 2.0

Popup {
    property var textArea
    property string emojiCategory: "faces"

    ColumnLayout {
        anchors.fill: parent

        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            cellWidth: 36
            cellHeight: 36

            boundsBehavior: Flickable.DragOverBounds

            clip: true

            model: XmlListModel {
                source: "qrc:/asset/xml/emoji.xml"
                query: "/root/emoji_by_category/" +emojiCategory + "/element"

                XmlRole { name: "emoji"; query: "string()" }
            }

            delegate: Label {
                width: 36
                height: 36

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pointSize: 20
                text: emoji

                MouseArea {
                    anchors.fill: parent
                    onClicked: textArea.insert(textArea.cursorPosition, emoji)
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: Material.theme == Material.Dark ? "white" : "black"
        }

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 48

            orientation: ListView.Horizontal

            boundsBehavior: Flickable.DragOverBounds

            clip: true

            model: XmlListModel {
                source: "qrc:/asset/xml/emoji.xml"
                query: "/root/emoji_categories/element"

                XmlRole { name: "emoji_unified"; query: "emoji_unified/string()" }
                XmlRole { name: "name"; query: "name/string()" }
            }

            delegate: Label {
                width: 48
                height: 48

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pointSize: 20
                text: emoji_unified

                MouseArea {
                    anchors.fill: parent
                    onClicked: emojiCategory = name
                }
            }

            ScrollBar.horizontal: ScrollBar {}
        }
    }
}
