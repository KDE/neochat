import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral 0.1
import Spectral.Effect 2.0
import Spectral.Component 2.0

Control {
    property alias avatar: headerImage.source
    property alias topic: headerTopicLabel.text
    property bool atTop: false
    signal clicked()

    id: header

    background: Rectangle {
        color: Material.background

        opacity: atTop ? 0 : 1

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 2
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12

        spacing: 12

        Avatar {
            Layout.preferredWidth: height
            Layout.fillHeight: true

            id: headerImage

            source: currentRoom.avatarUrl
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
                font.pixelSize: 16
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

    RippleEffect {
        anchors.fill: parent

        onClicked: header.clicked()
    }
}
