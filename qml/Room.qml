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

        connection: matriqueController.isLogin ? page.connection : undefined

        onNewMessage:  trayIcon.showMessage("New message", "New message for room " + room.displayName)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        RoomListForm {
            id: roomListForm

            Layout.fillHeight: true
            Layout.preferredWidth: settingPage.miniMode ? 80 : page.width * 0.4
            Layout.minimumWidth: 80
            Layout.maximumWidth: 360

            listModel:  roomListModel
        }

        RoomForm {
            id: roomForm

            Layout.fillWidth: true
            Layout.fillHeight: true

            currentRoom: roomListForm.currentIndex != -1 ? roomListModel.roomAt(roomListForm.currentIndex) : null
        }
    }
}
