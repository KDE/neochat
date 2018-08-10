import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1

import "qrc:/qml/form"

Page {
    property var connection

    id: page

    RoomListModel {
        id: roomListModel

        connection: page.connection

        onRoomAdded: setting.lazyLoad ? {} : room.getPreviousContent(20)
        onNewMessage: trayIcon.showMessage("New message", "New message for room " + room.displayName)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        RoomListForm {
            id: roomListForm

            Layout.fillHeight: true
            Layout.preferredWidth: setting.miniMode ? 80 : page.width * 0.35
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
