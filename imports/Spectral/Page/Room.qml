import QtQuick 2.9

RoomForm {
    roomListModel.onNewMessage: if (!window.visible) spectralController.showMessage(roomName, content, icon)
}
