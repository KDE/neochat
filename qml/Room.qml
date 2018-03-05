import QtQuick 2.10
import QtQuick.Controls 2.3

import "qrc:/qml/form"

import Matrique 0.1

Page {
    property RoomListModel roomListModel

    ListForm {
        id: roomListForm
        height: parent.height
        width: 320
        listModel: roomListModel
    }

    RoomForm {
        id: roomForm
        anchors.fill: parent
        anchors.leftMargin: roomListForm.width
        roomIndex: roomListForm.currentIndex
    }
}
