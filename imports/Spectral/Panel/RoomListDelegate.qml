import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0

Rectangle {
    color: MSettings.darkTheme ? "#303030" : "#fafafa"

    AutoMouseArea {
        anchors.fill: parent

        hoverEnabled: MSettings.miniMode

        onSecondaryClicked: {
            roomContextMenu.model = model
            roomContextMenu.popup()
        }
        onPrimaryClicked: {
            listView.currentIndex = index
            if (category === RoomType.Invited) {
                inviteDialog.currentRoom = currentRoom
                inviteDialog.open()
            } else {
                enteredRoom = currentRoom
            }
        }

        ToolTip.visible: MSettings.miniMode && containsMouse
        ToolTip.text: name
    }

    Rectangle {
        anchors.fill: parent

        visible: highlightCount > 0 || currentRoom === enteredRoom
        color: Material.accent
        opacity: 0.1
    }

    Rectangle {
        width: unreadCount > 0 ? 4 : 0
        height: parent.height

        color: Material.accent

        Behavior on width {
            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12

        spacing: 12

        ImageItem {
            id: imageItem

            Layout.preferredWidth: height
            Layout.fillHeight: true

            hint: name || "No Name"

            image: avatar
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            visible: parent.width > 64

            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: name || "No Name"
                font.pointSize: 12
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
            }

            Label {
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: (lastEvent == "" ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm,"")
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
            }
        }
    }
}
