import QtQuick 2.9

RoomForm {
    roomListModel.onNewMessage: if (!window.active) spectralController.showMessage(roomName, content, icon)
}
