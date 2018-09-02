import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1
import Matrique.Settings 0.1

import "form"

Page {
    property alias connection: roomListModel.connection

    id: page

    RoomListModel {
        id: roomListModel

        onRoomAdded: if (!MSettings.lazyLoad) room.getPreviousContent(20)
        onNewMessage: if (!window.active) matriqueController.showMessage(roomName, content, icon)
    }

    Rectangle {
        anchors.fill: parent

        color: MSettings.darkTheme ? "#363636" : "#f3f3f3"

        RowLayout {
            anchors.fill: parent

            spacing: 0

            RoomListForm {
                id: roomListForm

                Layout.fillHeight: true
                Layout.preferredWidth: MSettings.miniMode ? 64 : page.width * 0.35
                Layout.minimumWidth: 64
                Layout.maximumWidth: 360

                listModel:  roomListModel
            }

            RoomForm {
                id: roomForm

                Layout.fillWidth: true
                Layout.fillHeight: true

                currentRoom: roomListForm.enteredRoom
            }
        }
    }
}
