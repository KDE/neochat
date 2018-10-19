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
    property alias filter: roomListForm.filter

    property alias roomListModel: roomListModel
    property alias enteredRoom: roomListForm.enteredRoom

    id: page

    RoomListModel {
        id: roomListModel
    }

    RowLayout {
        anchors.fill: parent

        spacing: 0

        RoomListPanel {
            Layout.fillHeight: true
            Layout.preferredWidth: MSettings.miniMode ? 64 : page.width * 0.35
            Layout.minimumWidth: 64
            Layout.maximumWidth: 360

            id: roomListForm

            listModel: roomListModel

            layer.enabled: true
            layer.effect: ElevationEffect {
                elevation: 2
            }
        }

        RoomPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: roomForm

            currentRoom: roomListForm.enteredRoom
        }
    }
}


/*##^## Designer {
    D{i:0;autoSize:false;height:480;width:640}
}
 ##^##*/
