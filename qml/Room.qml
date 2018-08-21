import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1
import MatriqueSettings 0.1

import "qrc:/qml/form"

Page {
    property alias connection: roomListModel.connection

    id: page

    RoomListModel {
        id: roomListModel

        onRoomAdded: MatriqueSettings.lazyLoad ? {} : room.getPreviousContent(20)
        onNewMessage: window.active ? {} : matriqueController.showMessage(roomName, content, icon)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        RoomListForm {
            id: roomListForm

            Layout.fillHeight: true
            Layout.preferredWidth: MatriqueSettings.miniMode ? 80 : page.width * 0.35
            Layout.minimumWidth: 80
            Layout.maximumWidth: 360

            listModel:  roomListModel

            onEnterRoom: roomForm.currentRoom = currentRoom
        }

        RoomForm {
            id: roomForm

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
