import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral 0.1
import Spectral.Effect 2.0

Rectangle {
    property alias paintable: headerImage.source
    property alias topic: headerTopicLabel.text
    property bool atTop: false
    signal clicked()

    id: header

    color: atTop ? "transparent" : "white"

    layer.enabled: !atTop
    layer.effect: ElevationEffect {
        elevation: 4
    }

    ItemDelegate {
        anchors.fill: parent

        id: roomHeader

        onClicked: header.clicked()

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12

            spacing: 12

            ImageItem {
                Layout.preferredWidth: height
                Layout.fillHeight: true

                id: headerImage

                source: currentRoom.paintable
                hint: currentRoom ? currentRoom.displayName : "No name"
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                visible: parent.width > 64

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    text: currentRoom ? currentRoom.displayName : ""
                    color: "#1D333E"
                    font.pointSize: 12
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: headerTopicLabel

                    color: "#5B7480"
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }
            }
        }
    }
}
