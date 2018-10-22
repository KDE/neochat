import QtQuick 2.9

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral.Panel 2.0
import Spectral.Component 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1

Page {
    property alias connection: roomListModel.connection
    property alias enteredRoom: roomListForm.enteredRoom
    property alias filter: roomListForm.filter

    id: page

    RoomListModel {
        id: roomListModel

        onNewMessage: if (!window.active) spectralController.postNotification(roomId, eventId, roomName, senderName, text, icon, iconPath)
    }

    SplitView {
        anchors.fill: parent

        RoomListPanel {
//            Layout.fillHeight: true
            width: page.width * 0.35
            Layout.minimumWidth: 64
//            Layout.maximumWidth: 360

            id: roomListForm

            listModel: roomListModel

            onWidthChanged: {
                if (width < 240) width = 64
            }

            ElevationEffect {
                anchors.fill: source
                z: source.z - 1

                source: parent
                elevation: 2
            }
        }

        RoomPanel {
            Layout.fillWidth: true
            Layout.minimumWidth: 360
//            Layout.fillHeight: true

            id: roomForm

            currentRoom: roomListForm.enteredRoom
        }
    }
}
