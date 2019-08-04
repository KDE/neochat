import QtQuick 2.12
import QtQuick.Controls 2.12

import Spectral.Dialog 2.0

Menu {
    readonly property string selectedText: contentLabel.selectedText

    signal viewSource()
    signal reply()
    signal redact()

    id: root

    Item {
        width: parent.width
        height: 32

        Row {
            anchors.centerIn: parent

            spacing: 0

            Repeater {
                model: ["👍", "👎️", "😄", "🎉", "🚀", "👀"]

                delegate: ItemDelegate {
                    width: 32
                    height: 32

                    contentItem: Label {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font.pixelSize: 16
                        text: modelData
                    }

                    onClicked: currentRoom.toggleReaction(eventId, modelData)
                }
            }
        }
    }

    MenuSeparator {}

    MenuItem {
        text: "View Source"

        onTriggered: viewSource()
    }

    MenuItem {
        text: "Reply"

        onTriggered: reply()
    }

    MenuItem {
        text: "Redact"

        onTriggered: redact()
    }

    onClosed: destroy()
}
