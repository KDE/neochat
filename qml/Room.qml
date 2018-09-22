import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Spectral 0.1
import Spectral.Settings 0.1

import "form"

Page {
    property alias connection: roomListModel.connection
    property alias filter: roomListForm.filter

    id: page

    RoomListModel {
        id: roomListModel

        onRoomAdded: room.getPreviousContent(20)
        onNewMessage: if (!window.active) spectralController.showMessage(roomName, content, icon)
    }

    Rectangle {
        anchors.fill: parent

        color: MSettings.darkTheme ? "#323232" : "#f3f3f3"

        RowLayout {
            anchors.fill: parent

            spacing: 0

            RoomListForm {
                Layout.fillHeight: true
                Layout.preferredWidth: MSettings.miniMode ? 64 : page.width * 0.35
                Layout.minimumWidth: 64
                Layout.maximumWidth: 360

                id: roomListForm

                listModel:  roomListModel
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true

                color: MSettings.darkTheme ? "#363636" : "#ececec"
            }

            RoomForm {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: roomForm

                currentRoom: roomListForm.enteredRoom
            }
        }
    }
}
