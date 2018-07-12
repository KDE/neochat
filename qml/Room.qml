import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Matrique 0.1

import "qrc:/qml/form"

Page {
    property alias connection: roomListModel.connection

    id: page

    RoomListModel {
        id: roomListModel
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ListForm {
            id: roomListForm

            Layout.fillHeight: true
            Layout.preferredWidth:  {
                if (page.width > 560) {
                    return page.width * 0.4;
                } else {
                    return 80;
                }
            }
            Layout.maximumWidth: 360

            listModel: roomListModel
        }

        RoomForm {
            id: roomForm

            Layout.fillWidth: true
            Layout.fillHeight: true

            currentRoom: roomListForm.currentIndex != -1 ? roomListModel.roomAt(roomListForm.currentIndex) : null
        }
    }
}
