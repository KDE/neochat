import QtQuick 2.9

RoomForm {
    roomListModel.onNewMessage: if (!window.visible) spectralController.postNotification(roomId, eventId, roomName, senderName, text, icon, iconPath)
}
