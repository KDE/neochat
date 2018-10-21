import QtQuick 2.9

RoomForm {
    roomListModel.onNewMessage: if (!window.active) spectralController.postNotification(roomId, eventId, roomName, senderName, text, icon, iconPath)
}
