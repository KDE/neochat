import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import Spectral 0.1
import Spectral.Settings 0.1

import "form"
import "component"

Page {
    property alias connection: roomListModel.connection
    property alias filter: roomListForm.filter

    id: page

    RoomListModel {
        id: roomListModel

        onNewMessage: if (!window.active) spectralController.showMessage(roomName, content, icon)
    }

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

            layer.enabled: true
            layer.effect: ElevationEffect {
                elevation: 2
            }
        }

        RoomForm {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: roomForm

            currentRoom: roomListForm.enteredRoom
        }
    }
}
