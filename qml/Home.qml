import QtQuick 2.10
import QtQuick.Controls 2.3
import "qrc:/qml/form"

Page {
    RoomListForm {
        id: roomListForm
        height: parent.height
        width: 320
    }

    RoomForm {
        id: roomForm
        anchors.fill: parent
        anchors.leftMargin: roomListForm.width
    }
}
